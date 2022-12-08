#include "stdafx.h"

#include "drawing_canvas.h"

#include <cinder/ImageIo.h>
#include <cinder/Rand.h>
#include <cinder/Surface.h>
#include <cinder/gl/gl.h>

#include <Poco/LocalDateTime.h>

#include <ds/app/app.h>
#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/data/data_buffer.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/dirty_state.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/file_meta_data.h>

// #include <ds/gl/save_camera.h>

#include <thread>

namespace {

const static std::string whiteboard_point_vert = R"VERT(
uniform mat4		ciModelViewProjection;
in vec4			ciPosition;
in vec4			ciColor;
out vec4			oColor;
in vec2			ciTexCoord0;
out vec2			TexCoord0;
uniform vec4		vertexColor;
out vec4			brushColor;

void main(){
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0 = ciTexCoord0;
	oColor = ciColor;

	brushColor = vertexColor;
}
)VERT";

const static std::string whiteboard_point_frag = R"FRAG(
uniform sampler2D	tex0;
uniform float		opaccy;
in vec4			ciColor;
out vec4			oColor;
in vec2			TexCoord0;
in vec4			brushColor;
void main(){
oColor = texture2D(tex0, TexCoord0);
vec4 newColor = brushColor;
newColor.r *= brushColor.a * oColor.r;
newColor.g *= brushColor.a * oColor.g;
newColor.b *= brushColor.a * oColor.b;
newColor *= oColor.a;
oColor = newColor;
//oColor = brushColor;
// NEON EFFECTS!//gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0/2.2));
}
)FRAG";

static std::string whiteboard_point_name = "whiteboard_point";


const std::string opacityFrag = R"FRAG(
uniform sampler2D	tex0;
uniform float		opaccy;
in vec4			Color;
out vec4			oColor;
in vec2			TexCoord0;
void main()
{
//    oColor = vec4(1.0, 1.0, 1.0, 1.0);
    oColor = texture2D( tex0, TexCoord0 );
    oColor *= Color;
    oColor *= opaccy;
}
)FRAG";

const std::string vertShader = R"VERT(
uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
in vec4			ciColor;
out vec4			Color;
in vec2			ciTexCoord0;
out vec2			TexCoord0;
void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0 = ciTexCoord0;
	Color = ciColor;
}
)VERT";

std::string shaderNameOpaccy = "opaccy_shader";

	class Init {
	  public:
		Init() {
			ds::App::AddStartup([](ds::Engine& e) {
				e.installSprite([](ds::BlobRegistry& r) { ds::ui::DrawingCanvas::installAsServer(r); },
								[](ds::BlobRegistry& r) { ds::ui::DrawingCanvas::installAsClient(r); });
			});
		}
	};
	Init			  INIT;
	char			  BLOB_TYPE				= 0;
	const char		  DRAW_POINTS_QUEUE_ATT = 81;
	const char		  BRUSH_IMAGE_SRC_ATT	= 82;
	const char		  BRUSH_COLOR_ATT		= 83;
	const char		  BRUSH_SIZE_ATT		= 84;
	const char		  CANVAS_IMAGE_PATH_ATT = 85;
	const char		  CLEAR_CANVAS_ATT		= 86;
	const char		  ERASE_MODE_ATT		= 87;
	const ds::ui::DirtyState& sPointsQueueDirty		= ds::ui::newUniqueDirtyState();
	const ds::ui::DirtyState& sBrushColorDirty		= ds::ui::newUniqueDirtyState();
	const ds::ui::DirtyState& sBrushSizeDirty		= ds::ui::newUniqueDirtyState();
	const ds::ui::DirtyState& sCanvasImagePathDirty = ds::ui::newUniqueDirtyState();
	const ds::ui::DirtyState& sClearCanvasDirty		= ds::ui::newUniqueDirtyState();
	const ds::ui::DirtyState& sEraseModeDirty		= ds::ui::newUniqueDirtyState();

	const int MAX_SERIALIZED_POINTS = 100;
} // namespace

namespace ds::ui {

// Client/Server Stuff ------------------------------

void DrawingCanvas::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
}

void DrawingCanvas::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<DrawingCanvas>(r); });
}

// -- Client/Server Stuff ----------------------------


DrawingCanvas::DrawingCanvas(ds::ui::SpriteEngine& eng, const std::string& brushImagePath)
  : ds::ui::Sprite(eng)
  , mCanvasFileLoaderClient(eng)
  , mPointShader(whiteboard_point_vert, whiteboard_point_frag, whiteboard_point_name)
  , mBrushImage(nullptr)
  , mBrushSize(24.0f)
  , mBrushColor(1.0f, 0.0f, 0.0f, 0.5f)
  , mEraseMode(false) {

	mBlobType = BLOB_TYPE;
	setBaseShader(vertShader, opacityFrag, shaderNameOpaccy);

	mBrushImage = new Image(mEngine);
	addChildPtr(mBrushImage);
	mBrushImage->hide();

	mPointShader.loadShaders();

	DS_REPORT_GL_ERRORS();

	setBrushImage(brushImagePath);
	markAsDirty(sBrushSizeDirty);

	setBrushColor(ci::ColorA(1.0f, 0.3f, 0.3f, 0.7f));
	setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	setTransparent(false);
	setColor(ci::Color(1.0f, 1.0f, 1.0f));
	setUseShaderTexture(true);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti) {
		auto localPoint = globalToLocal(ti.mCurrentGlobalPoint);
		auto prevPoint	= globalToLocal(ti.mCurrentGlobalPoint - ti.mDeltaPoint);

		if (ti.mPhase == ds::ui::TouchInfo::Added) {
			mSerializedPointsQueue.push_back(std::make_pair(ci::vec2(localPoint), ci::vec2(localPoint)));
			renderLine(localPoint, localPoint);
			markAsDirty(sPointsQueueDirty);
			mCurrentLine.push_back(std::make_pair(ci::vec2(localPoint), ci::vec2(localPoint)));

			if (ti.mNumberFingers == 1) {
				mTouchHolding		= true;
				mTouchHoldStartPos	= ti.mStartPoint;
				mTouchHoldStartTime = mEngine.getElapsedTimeSeconds();
			} else {
				mTouchHolding = false;
			}
		}
		if (ti.mPhase == ds::ui::TouchInfo::Moved) {
			mSerializedPointsQueue.push_back(std::make_pair(ci::vec2(prevPoint), ci::vec2(localPoint)));
			renderLine(prevPoint, localPoint);
			markAsDirty(sPointsQueueDirty);
			mCurrentLine.push_back(std::make_pair(ci::vec2(prevPoint), ci::vec2(localPoint)));

			if (glm::distance(ti.mCurrentGlobalPoint, mTouchHoldStartPos) > mEngine.getMinTapDistance()) {
				mTouchHolding = false;
			}
		}
		if (ti.mNumberFingers <= 0) {
			if (mTouchHolding &&
				glm::distance(ti.mCurrentGlobalPoint, mTouchHoldStartPos) <= mEngine.getMinTapDistance() &&
				mEngine.getElapsedTimeSeconds() - mTouchHoldStartTime <
					mEngine.getAppSettings().getDouble("touch:hold_time", 0, 2.0)) {
				mTouchHolding = false;
			}
			if (mCompleteLineCallback && !mCurrentLine.empty()) {
				mCompleteLineCallback(mCurrentLine);
				mCurrentLine.clear();
			}
		}
		// Don't let the queue get too large if there are no clients connected
		while (mSerializedPointsQueue.size() > MAX_SERIALIZED_POINTS) {
			mSerializedPointsQueue.pop_front();
		}
	});
}


void DrawingCanvas::onUpdateServer(const ds::UpdateParams& updateParams) {
	if (!isEnabled()) mTouchHolding = false;
	if (mTouchHolding && mTouchHoldCallback &&
		mEngine.getElapsedTimeSeconds() - mTouchHoldStartTime >=
			mEngine.getAppSettings().getDouble("touch:hold_time", 0, 2.0)) {
		if (mCompleteLineCallback && !mCurrentLine.empty()) {
			mCompleteLineCallback(mCurrentLine);
			mCurrentLine.clear();
		}
		mTouchHoldCallback(mTouchHoldStartPos);
	}
}

void DrawingCanvas::setBrushColor(const ci::ColorA& brushColor) {
	mBrushColor = brushColor;
	markAsDirty(sBrushColorDirty);
}

void DrawingCanvas::setBrushColor(const ci::Color& brushColor) {
	mBrushColor.r = brushColor.r;
	mBrushColor.g = brushColor.g;
	mBrushColor.b = brushColor.b;
	markAsDirty(sBrushColorDirty);
}

void DrawingCanvas::setBrushOpacity(const float brushOpacity) {
	mBrushColor.a = brushOpacity;
	markAsDirty(sBrushColorDirty);
}

const ci::ColorA& DrawingCanvas::getBrushColor() {
	return mBrushColor;
}

void DrawingCanvas::setBrushSize(const float brushSize) {
	mBrushSize = brushSize;
	markAsDirty(sBrushSizeDirty);
}

const float DrawingCanvas::getBrushSize() {
	return mBrushSize;
}

void DrawingCanvas::setBrushImage(const std::string& imagePath) {
	if (!mBrushImage) return;
	DS_LOG_VERBOSE(3, "DrawingCanvas: setBrushImage " << imagePath);

	mBrushImagePath = imagePath;

	if (imagePath.empty()) {
		mBrushImage->clearImage();
	} else {
		mBrushImage->setImageFile(imagePath);
	}
}

ci::gl::Texture2dRef DrawingCanvas::getBrushImageTexture() {
	if (mBrushImage) {
		return mBrushImage->getImageTexture();
	}

	return ci::gl::Texture2dRef();
}

ci::gl::FboRef DrawingCanvas::getDrawingFbo() {
	createFbo();
	return mFbo;
}

void DrawingCanvas::clearCanvas() {

	DS_LOG_VERBOSE(3, "DrawingCanvas: clearCanvas");

	if (!mFbo) return;

	ci::gl::ScopedFramebuffer fbScp(mFbo);

	ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	if (mEngine.getMode() == ds::ui::SpriteEngine::SERVER_MODE ||
		mEngine.getMode() == ds::ui::SpriteEngine::CLIENTSERVER_MODE) {
		markAsDirty(sClearCanvasDirty);
	}
}

void DrawingCanvas::setEraseMode(const bool eraseMode) {
	mEraseMode = eraseMode;
	markAsDirty(sEraseModeDirty);
}

bool DrawingCanvas::getEraseMode() {
	return mEraseMode;
}

void DrawingCanvas::drawLocalClient() {
	// If we have a new texture from the canvas loader,
	// swap that in for the draw texture
	if (mCanvasFileLoaderClient.getImageTexture()) {
		auto loaderTex = mCanvasFileLoaderClient.getImageTexture();
		// TODO
		///	mDrawTexture = *loaderTex;
		mCanvasFileLoaderClient.clearImage();
	}

	// If any serialized points have been received from the server, draw them
	while (!mSerializedPointsQueue.empty()) {
		auto points = mSerializedPointsQueue.front();
		renderLine(ci::vec3(points.first, 0), ci::vec3(points.second, 0));
		mSerializedPointsQueue.pop_front();
	}

	if (mFbo) {

		// ignore the "color" setting from base sprite
		ci::gl::color(ci::Color::white());
		ci::gl::GlslProgRef shaderBase = getBaseShader().getShader();

		auto theTex = mFbo->getTexture2d(GL_COLOR_ATTACHMENT0);
		theTex->bind(0);

		if (shaderBase) {
			ci::gl::ScopedBlend sb(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			shaderBase->uniform("tex0", 0);
			shaderBase->uniform("opaccy", mDrawOpacity);

			if (!getPerspective()) {
				if (mRenderBatch) {
					mRenderBatch->draw();
				} else {
					ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mFbo->getWidth()),
													static_cast<float>(mFbo->getHeight())));
				}
			} else {
				ci::gl::drawSolidRect(
					ci::Rectf(0.0f, static_cast<float>(mFbo->getHeight()), static_cast<float>(mFbo->getWidth()), 0.0f));
			}

		} else {

			if (!getPerspective()) {
				ci::gl::drawSolidRect(
					ci::Rectf(0.0f, 0.0f, static_cast<float>(mFbo->getWidth()), static_cast<float>(mFbo->getHeight())));
			} else {
				ci::gl::drawSolidRect(
					ci::Rectf(0.0f, static_cast<float>(mFbo->getHeight()), static_cast<float>(mFbo->getWidth()), 0.0f));
			}
		}
	}
}

void DrawingCanvas::createFbo() {
	auto w = (int)floorf(getWidth());
	auto h = (int)floorf(getHeight());
	if (!mFbo || mFbo->getWidth() != w || mFbo->getHeight() != h) {
		ci::gl::Texture2d::Format textFormat;
		textFormat.setMinFilter(GL_LINEAR);
		textFormat.setMagFilter(GL_LINEAR);
		textFormat.setInternalFormat(GL_RGBA32F);

		ci::gl::Fbo::Format format;
		// format.setSamples(4); // NOTE: don't anti-alias, it causes some weird shit at the edges
		format.attachment(GL_COLOR_ATTACHMENT0, ci::gl::Texture2d::create(w, h, textFormat));
		mFbo = ci::gl::Fbo::create(w, h, format);
	}
}

void DrawingCanvas::renderLine(const ci::vec3& start, const ci::vec3& end) {

	if (!mBrushImage) {
		DS_LOG_WARNING("No brush image sprite in drawing canvas");
		return;
	}

	if (mRenderLineCallback) {
		mRenderLineCallback(std::make_pair(ci::vec2(start), ci::vec2(end)));
	}

	DS_LOG_VERBOSE(5, "DrawingCanvas: renderLine start=" << start << " end=" << end);

	ci::gl::Texture2dRef brushTexture = mBrushImage->getImageTexture();

	bool  brushTexMode = true;
	float widdy		   = mBrushSize;
	float hiddy		   = mBrushSize;

	if (brushTexture) {
		brushTexture->setTopDown(true);
		hiddy = mBrushSize / ((float)brushTexture->getWidth() / (float)brushTexture->getHeight());
	} else {
		brushTexMode = false;
	}

	float brushPixelStep = 3.0f;

	std::vector<ci::vec2> drawPoints;

	// Create a point for every pixel between start and end for smoothness
	int count =
		std::max<int>((int)ceilf(sqrtf((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y)) /
								 (float)brushPixelStep),
					  1);
	for (int i = 0; i < count; ++i) {
		drawPoints.push_back(ci::vec2(start.x + (end.x - start.x) * ((float)i / (float)count),
									  start.y + (end.y - start.y) * ((float)i / (float)count)));
	}

	createFbo();

	{
		ci::gl::pushMatrices();
		ci::gl::ScopedFramebuffer fbScp(mFbo);
		ci::gl::ScopedViewport	  scpVp(ci::ivec2(0), mFbo->getSize());

		ci::CameraOrtho camera = ci::CameraOrtho(0.0f, static_cast<float>(mFbo->getWidth()),
												 static_cast<float>(mFbo->getHeight()), 0.0f, -1000.0f, 1000.0f);
		ci::gl::setMatrices(camera);

		int	 blendSfactor = 0;
		auto drawColor	  = mBrushColor;

		if (mEraseMode) {
			blendSfactor = GL_ZERO;
		} else {
			blendSfactor = GL_ONE;
		}

		ci::gl::ScopedBlend enableBlend(true);
		ci::gl::ScopedBlend enableFunc(blendSfactor, GL_ONE_MINUS_SRC_ALPHA);


		for (auto it : drawPoints) {
			ci::Rectf destRect =
				ci::Rectf(it.x - widdy / 2.0f, it.y - hiddy / 2.0f, it.x + widdy / 2.0f, it.y + hiddy / 2.0f);

			if (brushTexMode) {
				mPointShader.getShader()->uniform("tex0", 0);
				mPointShader.getShader()->uniform("vertexColor", drawColor);
				ci::gl::ScopedGlslProg shaderScp(mPointShader.getShader());
				brushTexture->bind(0);
				ci::gl::drawSolidRect(destRect);
			} else {
				ci::gl::ScopedGlslProg shaderScp(ci::gl::getStockShader(ci::gl::ShaderDef().color()));
				ci::gl::color(mBrushColor);
				ci::gl::drawSolidCircle(it, mBrushSize / 2.0f);
			}
		}

		ci::gl::popMatrices();
	}

	DS_REPORT_GL_ERRORS();
}

void DrawingCanvas::writeAttributesTo(DataBuffer& buf) {
	Sprite::writeAttributesTo(buf);

	if (mDirty.has(sBrushColorDirty)) {
		buf.add(BRUSH_COLOR_ATT);
		buf.add(mBrushColor.r);
		buf.add(mBrushColor.g);
		buf.add(mBrushColor.b);
		buf.add(mBrushColor.a);
	}
	if (mDirty.has(sBrushSizeDirty)) {
		buf.add(BRUSH_SIZE_ATT);
		buf.add<float>(mBrushSize);
	}
	if (mDirty.has(sPointsQueueDirty)) {
		buf.add(DRAW_POINTS_QUEUE_ATT);
		buf.add<uint32_t>((uint32_t)mSerializedPointsQueue.size());
		for (auto& pair : mSerializedPointsQueue) {
			buf.add<float>(pair.first.x);
			buf.add<float>(pair.first.y);
			buf.add<float>(pair.second.x);
			buf.add<float>(pair.second.y);
		}
		mSerializedPointsQueue.clear();
	}
	if (mDirty.has(sCanvasImagePathDirty)) {
		// buf.add(CANVAS_IMAGE_PATH_ATT);
		// mCanvasFileLoaderClient.writeTo(buf);
	}
	if (mDirty.has(sClearCanvasDirty)) {
		buf.add(CLEAR_CANVAS_ATT);
	}
	if (mDirty.has(sEraseModeDirty)) {
		buf.add(ERASE_MODE_ATT);
		buf.add<bool>(mEraseMode);
	}
}

void DrawingCanvas::readAttributeFrom(const char attrid, DataBuffer& buf) {
	if (attrid == BRUSH_COLOR_ATT) {
		mBrushColor.r = buf.read<float>();
		mBrushColor.g = buf.read<float>();
		mBrushColor.b = buf.read<float>();
		mBrushColor.a = buf.read<float>();
	} else if (attrid == BRUSH_SIZE_ATT) {
		mBrushSize = buf.read<float>();
	} else if (attrid == DRAW_POINTS_QUEUE_ATT) {
		uint32_t count = buf.read<uint32_t>();
		ci::vec2 p1, p2;
		for (uint32_t i = 0; i < count; i++) {
			p1.x = buf.read<float>();
			p1.y = buf.read<float>();
			p2.x = buf.read<float>();
			p2.y = buf.read<float>();
			mSerializedPointsQueue.push_back(std::make_pair(p1, p2));
		}
	} else if (attrid == CANVAS_IMAGE_PATH_ATT) {
		//	mCanvasFileLoaderClient.readFrom(buf);
	} else if (attrid == CLEAR_CANVAS_ATT) {
		clearCanvas();
	} else if (attrid == ERASE_MODE_ATT) {
		mEraseMode = buf.read<bool>();
	} else {
		Sprite::readAttributeFrom(attrid, buf);
	}
}

void DrawingCanvas::saveCanvasImage(const std::string& filePath) {

	if (!(mFbo && mFbo->getWidth() > 0 && mFbo->getHeight() > 0)) return;

	/* TODO: Test this, it should be updated 0.9.0 */
	// This can't be done on the background thread because it needs
	// the main thread's GL context to get the texture data.
	ci::Surface surface(
		mFbo->readPixels8u(ci::Area(0, 0, mFbo->getWidth(), mFbo->getHeight()))); // TODO (mDrawTexture);

	// Do the image file saving on background thread
	auto saveThread = std::thread([surface, filePath] {
		try {
			ci::writeImage(filePath, surface);
		} catch (const std::exception& e) {
			DS_LOG_WARNING("DrawingCanvas: Unable to save canvas to file: " << filePath << ": " << e.what());
		}
	});

	// We don't care about this thread anymore, it can terminate on
	// its own. But we do need to detach if we're not going to join
	// the thread.
	saveThread.detach();
}

void DrawingCanvas::loadCanvasImage(const std::string& filePath) {
	markAsDirty(sCanvasImagePathDirty);

	DS_LOG_WARNING("Re-implement DrawingCanvas::loadCanvasImage()!");
	// This will load the image file asynchronously.  When it's
	// ready, the texture will be grabbed in drawLocalClient.
	// mCanvasFileLoaderClient.setSource(ds::ui::ImageFile(filePath));
}


} // namespace ds::ui
