#include "stdafx.h"

#include "drawing_canvas.h"

#include <cinder/gl/gl.h>
#include <cinder/Surface.h>

#include <Poco/LocalDateTime.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_registry.h>
#include <ds/app/blob_reader.h>
#include "ds/data/data_buffer.h"
#include <ds/ui/sprite/dirty_state.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/image_source/image_file.h>

#include <ds/debug/logger.h>
#include <ds/util/file_meta_data.h>
#include <ds/app/environment.h>

//#include <ds/gl/save_camera.h>
#include <cinder/ImageIo.h>

#include <cinder/Rand.h>

#include <thread>

namespace {

const static std::string whiteboard_point_vert =
"#version 150\n"
"uniform mat4		ciModelViewProjection;\n"
"in vec4			ciPosition;\n"
"in vec4			ciColor;\n"
"out vec4			oColor;\n"
"in vec2			ciTexCoord0;\n"
"out vec2			TexCoord0;\n"
"uniform vec4		vertexColor;\n"
"out vec4			brushColor;\n"

"void main(){\n"
"	gl_Position = ciModelViewProjection * ciPosition;\n"
"	TexCoord0 = ciTexCoord0;\n"
"	oColor = ciColor;\n"

	"brushColor = vertexColor;\n"
"}\n";

const static std::string whiteboard_point_frag =
"uniform sampler2D	tex0;\n"
"uniform float		opaccy;\n"
"in vec4			ciColor;\n"
"out vec4			oColor;\n"
"in vec2			TexCoord0;\n"
"in vec4			brushColor;\n"
"void main(){\n"
"oColor = texture2D(tex0, TexCoord0);\n"
"vec4 newColor = brushColor;\n"
"newColor.r *= brushColor.a * oColor.r;\n"
"newColor.g *= brushColor.a * oColor.g;\n"
"newColor.b *= brushColor.a * oColor.b;\n"
"newColor *= oColor.a;\n"
"oColor = newColor;\n"
//"oColor = brushColor;\n"
//NEON EFFECTS!//"gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0/2.2));"
"}\n";

static std::string whiteboard_point_name = "whiteboard_point";


const std::string opacityFrag =
"uniform sampler2D	tex0;\n"
"uniform float		opaccy;\n"
"in vec4			Color;\n"
"out vec4			oColor;\n"
"in vec2			TexCoord0;\n"
"void main()\n"
"{\n"
//"    oColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    oColor = texture2D( tex0, TexCoord0 );\n"
"    oColor *= Color;\n"
"    oColor *= opaccy;\n"
"}\n";

const std::string vertShader =
"uniform mat4	ciModelViewProjection;\n"
"in vec4			ciPosition;\n"
"in vec4			ciColor;\n"
"out vec4			Color;\n"
"in vec2			ciTexCoord0;\n"
"out vec2			TexCoord0;\n"
"void main()\n"
"{\n"
"	gl_Position = ciModelViewProjection * ciPosition;\n"
"	TexCoord0 = ciTexCoord0;\n"
"	Color = ciColor;\n"
"}\n";

std::string shaderNameOpaccy = "opaccy_shader";
}

namespace ds {
namespace ui {

// Client/Server Stuff ------------------------------
namespace { // anonymous namespace
class Init {
public:
	Init() {
		ds::App::AddStartup( []( ds::Engine& e ) {
			e.installSprite(	[]( ds::BlobRegistry& r ){ds::ui::DrawingCanvas::installAsServer( r ); },
								[]( ds::BlobRegistry& r ){ds::ui::DrawingCanvas::installAsClient( r ); } );
		} );
	}
};
Init				INIT;
char				BLOB_TYPE				= 0;
const char			DRAW_POINTS_QUEUE_ATT	= 81;
const char			BRUSH_IMAGE_SRC_ATT		= 82;
const char			BRUSH_COLOR_ATT 		= 83;
const char			BRUSH_SIZE_ATT			= 84;
const char			CANVAS_IMAGE_PATH_ATT	= 85;
const char			CLEAR_CANVAS_ATT		= 86;
const char			ERASE_MODE_ATT			= 87;
const DirtyState&	sPointsQueueDirty	 	= newUniqueDirtyState();
const DirtyState&	sBrushImagePathDirty	= newUniqueDirtyState();
const DirtyState&	sBrushColorDirty		= newUniqueDirtyState();
const DirtyState&	sBrushSizeDirty			= newUniqueDirtyState();
const DirtyState&	sCanvasImagePathDirty	= newUniqueDirtyState();
const DirtyState&	sClearCanvasDirty		= newUniqueDirtyState();
const DirtyState&	sEraseModeDirty			= newUniqueDirtyState();

const int			MAX_SERIALIZED_POINTS	= 100;
} // anonymous namespace

void DrawingCanvas::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void DrawingCanvas::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<DrawingCanvas>(r); });
}
// -- Client/Server Stuff ----------------------------


DrawingCanvas::DrawingCanvas(ds::ui::SpriteEngine& eng, const std::string& brushImagePath)
	: ds::ui::Sprite(eng)
	, ds::ui::ImageOwner(eng)
	, mBrushSize(24.0f)
	, mBrushColor(1.0f, 0.0f, 0.0f, 0.5f)
	, mPointShader(whiteboard_point_vert, whiteboard_point_frag, whiteboard_point_name)
	, mEraseMode(false)
	, mCanvasFileLoaderClient(eng)
{
	mBlobType = BLOB_TYPE;
	setBaseShader(vertShader, opacityFrag, shaderNameOpaccy);

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
	setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti){
		auto localPoint = globalToLocal(ti.mCurrentGlobalPoint);
		auto prevPoint = globalToLocal(ti.mCurrentGlobalPoint - ti.mDeltaPoint);
		if(ti.mPhase == ds::ui::TouchInfo::Added){
			//TODO: make sure it is necessary to make_pair
			mSerializedPointsQueue.push_back( std::make_pair(ci::vec2(localPoint), ci::vec2(localPoint)));
			renderLine(localPoint, localPoint);
			markAsDirty(sPointsQueueDirty);
		}
		if(ti.mPhase == ds::ui::TouchInfo::Moved){
			mSerializedPointsQueue.push_back( std::make_pair(ci::vec2(prevPoint), ci::vec2(localPoint)) );
			renderLine(prevPoint, localPoint);
			markAsDirty(sPointsQueueDirty);
		}
		// Don't let the queue get too large if there are no clients connected
		while (mSerializedPointsQueue.size() > MAX_SERIALIZED_POINTS)
			mSerializedPointsQueue.pop_front();
	});
}

void DrawingCanvas::setBrushColor(const ci::ColorA& brushColor){
	mBrushColor = brushColor;
	markAsDirty(sBrushColorDirty);
}

void DrawingCanvas::setBrushColor(const ci::Color& brushColor){
	mBrushColor.r = brushColor.r; mBrushColor.g = brushColor.g; mBrushColor.b = brushColor.b;
	markAsDirty(sBrushColorDirty);
}

void DrawingCanvas::setBrushOpacity(const float brushOpacity){
	mBrushColor.a = brushOpacity;
	markAsDirty(sBrushColorDirty);
}

const ci::ColorA& DrawingCanvas::getBrushColor(){
	return mBrushColor;
}

void DrawingCanvas::setBrushSize(const float brushSize){
	mBrushSize = brushSize;
	markAsDirty(sBrushSizeDirty);
}

const float DrawingCanvas::getBrushSize(){
	return mBrushSize;
}

void DrawingCanvas::setBrushImage(const std::string& imagePath){
	mBrushImagePath = imagePath;

	if (imagePath.empty()){
		clearImage();
	} else {
		setImageFile(imagePath);

		// The image won't actually load until it's requested, so try to load it immediately
		getImageTexture();
	}
	markAsDirty( sBrushImagePathDirty );
}

void DrawingCanvas::clearCanvas(){
	auto w = getWidth();
	auto h = getHeight();

	if(!mFbo) return;

	ci::gl::ScopedFramebuffer fbScp(mFbo);

	ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	if( mEngine.getMode() == ds::ui::SpriteEngine::SERVER_MODE ||
		mEngine.getMode() == ds::ui::SpriteEngine::CLIENTSERVER_MODE
	) {
		markAsDirty( sClearCanvasDirty );
	}
}

void DrawingCanvas::setEraseMode(const bool eraseMode){
	mEraseMode = eraseMode;
	markAsDirty(sEraseModeDirty);
}

void DrawingCanvas::drawLocalClient(){
	// If we have a new texture from the canvas loader,
	// swap that in for the draw texture
	if (mCanvasFileLoaderClient.getImage()) {
		auto loaderTex = mCanvasFileLoaderClient.getImage();
		// TODO
		//mDrawTexture = *loaderTex;
		mCanvasFileLoaderClient.clear();
	}

	// If any serialized points have been received from the server, draw them
	while (!mSerializedPointsQueue.empty()) {
		auto points = mSerializedPointsQueue.front();
		renderLine( ci::vec3( points.first,0 ), ci::vec3( points.second,0 ) );
		mSerializedPointsQueue.pop_front();
	}

	if(mFbo) {

		// ignore the "color" setting from base sprite
		ci::gl::color(ci::Color::white());
		ci::gl::GlslProgRef shaderBase = getBaseShader().getShader();

		auto theTex = mFbo->getTexture2d(GL_COLOR_ATTACHMENT1);
		theTex->bind(0);

		if(shaderBase) {
			//ci::gl::enableAlphaBlending(true);
			ci::gl::ScopedBlend sb(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			shaderBase->uniform("tex0", 0);
			shaderBase->uniform("opaccy", mDrawOpacity);
			//ci::gl::ScopedGlslProg scopedShaderBase(shaderBase);

			if(!getPerspective()){
				if(mRenderBatch){
					mRenderBatch->draw();
				} else {
					ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mFbo->getWidth()), static_cast<float>(mFbo->getHeight())));
				}
			} else {
				ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mFbo->getHeight()), static_cast<float>(mFbo->getWidth()), 0.0f));
			}

		} else {

			if (!getPerspective()){
				ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mFbo->getWidth()), static_cast<float>(mFbo->getHeight())));
			} else {
				ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mFbo->getHeight()), static_cast<float>(mFbo->getWidth()), 0.0f));
			}
		}
	}
}

void DrawingCanvas::renderLine(const ci::vec3& start, const ci::vec3& end){
	ci::gl::Texture2dRef brushTexture = getImageTexture();

	bool brushTexMode = true;
	float widdy = mBrushSize;
	float hiddy = mBrushSize;
	
	if(brushTexture){
		brushTexture->setTopDown(true);
		hiddy = mBrushSize / ((float)brushTexture->getWidth() / (float)brushTexture->getHeight());
	} else {
		brushTexMode = false;
	}

	float brushPixelStep = 3.0f;
	int vertexCount = 0;

	std::vector<ci::vec2> drawPoints;

	// Create a point for every pixel between start and end for smoothness
	int count = std::max<int>((int)ceilf(sqrtf((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y)) / (float)brushPixelStep), 1);
	for(int i = 0; i < count; ++i) {
		drawPoints.push_back(ci::vec2(start.x + (end.x - start.x) * ((float)i / (float)count), start.y + (end.y - start.y) * ((float)i / (float)count)));
	}

	int w = (int)floorf(getWidth());
	int h = (int)floorf(getHeight());

	if(!mFbo || mFbo->getWidth() != w || mFbo->getHeight() != h) {
		ci::gl::Texture2d::Format textFormat;
		textFormat.setMinFilter(GL_LINEAR);
		textFormat.setMagFilter(GL_LINEAR);
		textFormat.setInternalFormat(GL_RGBA32F);
		ci::gl::Fbo::Format format;
		//format.setSamples(4); // NOTE: don't anti-alias, it causes some weird shit at the edges
		format.attachment(GL_COLOR_ATTACHMENT1, ci::gl::Texture2d::create(w, h, textFormat));
		mFbo = ci::gl::Fbo::create(w, h, format);
	}

	{
		ci::gl::pushMatrices();
		ci::gl::ScopedFramebuffer fbScp(mFbo);
		ci::gl::ScopedViewport scpVp(ci::ivec2(0), mFbo->getSize());

		ci::CameraOrtho camera = ci::CameraOrtho(0.0f, static_cast<float>(mFbo->getWidth()), static_cast<float>(mFbo->getHeight()), 0.0f, -1000.0f, 1000.0f);
		ci::gl::setMatrices(camera);

		int blendSfactor = 0;
		auto drawColor = mBrushColor;

		if(mEraseMode){
			blendSfactor = GL_ZERO;
		} else {
			blendSfactor = GL_ONE;
		}

		ci::gl::ScopedBlend enableBlend(true);
		ci::gl::ScopedBlend enableFunc(blendSfactor, GL_ONE_MINUS_SRC_ALPHA);


		for(auto it : drawPoints){
			ci::Rectf destRect = ci::Rectf(it.x - widdy / 2.0f, it.y - hiddy / 2.0f, it.x + widdy / 2.0f, it.y + hiddy / 2.0f);

			if(brushTexMode){
				mPointShader.getShader()->uniform("tex0", 0);
				mPointShader.getShader()->uniform("vertexColor", drawColor);
				ci::gl::ScopedGlslProg shaderScp(mPointShader.getShader());
				brushTexture->bind(0);
				ci::gl::drawSolidRect(destRect);
			} else {
				ci::gl::ScopedGlslProg shaderScp(ci::gl::getStockShader(ci::gl::ShaderDef().color()));
				ci::gl::color(mBrushColor);
				ci::gl::drawSolidCircle(it, mBrushSize/2.0f);
			}
		}

		ci::gl::popMatrices();
	}


	DS_REPORT_GL_ERRORS();
}

void DrawingCanvas::writeAttributesTo(DataBuffer& buf) {
	Sprite::writeAttributesTo(buf);

	if (mDirty.has(sBrushImagePathDirty)){
		buf.add(BRUSH_IMAGE_SRC_ATT);
		mImageSource.writeTo(buf);
	}
	if (mDirty.has(sBrushColorDirty)){
		buf.add(BRUSH_COLOR_ATT);
		buf.add(mBrushColor.r);
		buf.add(mBrushColor.g);
		buf.add(mBrushColor.b);
		buf.add(mBrushColor.a);
	}
	if (mDirty.has(sBrushSizeDirty)){
		buf.add(BRUSH_SIZE_ATT);
		buf.add<float>(mBrushSize);
	}
	if (mDirty.has(sPointsQueueDirty)){
		buf.add(DRAW_POINTS_QUEUE_ATT);
		buf.add<uint32_t>((uint32_t)mSerializedPointsQueue.size());
		for( auto &pair : mSerializedPointsQueue ) {
			buf.add<float>(pair.first.x);
			buf.add<float>(pair.first.y);
			buf.add<float>(pair.second.x);
			buf.add<float>(pair.second.y);
		}
		mSerializedPointsQueue.clear();
	}
	if (mDirty.has(sCanvasImagePathDirty)){
		buf.add(CANVAS_IMAGE_PATH_ATT);
		mCanvasFileLoaderClient.writeTo(buf);
	}
	if (mDirty.has(sClearCanvasDirty)){
		buf.add(CLEAR_CANVAS_ATT);
	}
	if (mDirty.has(sEraseModeDirty)){
		buf.add(ERASE_MODE_ATT);
		buf.add<bool>(mEraseMode);
	}
}

void DrawingCanvas::readAttributeFrom(const char attrid, DataBuffer& buf){
	if (attrid == BRUSH_IMAGE_SRC_ATT) {
		mImageSource.readFrom(buf);
	}
	else if (attrid == BRUSH_COLOR_ATT) {
		mBrushColor.r = buf.read<float>();
		mBrushColor.g = buf.read<float>();
		mBrushColor.b = buf.read<float>();
		mBrushColor.a = buf.read<float>();
	}
	else if (attrid == BRUSH_SIZE_ATT) {
		mBrushSize = buf.read<float>();
	}
	else if (attrid == DRAW_POINTS_QUEUE_ATT) {
		uint32_t count = buf.read<uint32_t>();
		ci::vec2 p1, p2;
		for (uint32_t i = 0; i<count; i++) {
			p1.x = buf.read<float>();
			p1.y = buf.read<float>();
			p2.x = buf.read<float>();
			p2.y = buf.read<float>();
			mSerializedPointsQueue.push_back( std::make_pair(p1, p2) );
		}
	}
	else if (attrid == CANVAS_IMAGE_PATH_ATT) {
		mCanvasFileLoaderClient.readFrom(buf);
	}
	else if (attrid == CLEAR_CANVAS_ATT) {
		clearCanvas();
	}
	else if (attrid == ERASE_MODE_ATT) {
		mEraseMode = buf.read<bool>();
	}
	else {
		Sprite::readAttributeFrom(attrid, buf);
	}
}

void DrawingCanvas::onImageChanged() {
	markAsDirty(sBrushImagePathDirty);
}

void DrawingCanvas::saveCanvasImage(const std::string& filePath) {

	if(!(mFbo && mFbo->getWidth() > 0 && mFbo->getHeight() > 0))
			return;

		/* TODO: Test this, it should be updated 0.9.0 */
		// This can't be done on the background thread because it needs
		// the main thread's GL context to get the texture data.
	ci::Surface surface(mFbo->readPixels8u(ci::Area(0, 0, mFbo->getWidth(), mFbo->getHeight())));//TODO (mDrawTexture);

	// Do the image file saving on background thread
	auto saveThread = std::thread([surface, filePath] {
		try {
			ci::writeImage(filePath, surface);
		}
		catch (const std::exception &e) {
			DS_LOG_WARNING( "DrawingCanvas: Unable to save canvas to file: " << filePath << ": " << e.what() );
		}
	});

	// We don't care about this thread anymore, it can terminate on 
	// its own. But we do need to detach if we're not going to join 
	// the thread.
	saveThread.detach();
	
}

void DrawingCanvas::loadCanvasImage(const std::string& filePath) {
	markAsDirty(sCanvasImagePathDirty);

	// This will load the image file asynchronously.  When it's
	// ready, the texture will be grabbed in drawLocalClient.
	mCanvasFileLoaderClient.setSource(ds::ui::ImageFile(filePath));
}


} // namespace ui
} // namespace ds
