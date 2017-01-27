#include "drawing_canvas.h"

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

#include <ds/gl/save_camera.h>
#include <cinder/ImageIo.h>

#include <cinder/Rand.h>

#include <thread>

namespace {

const static std::string whiteboard_point_vert =
"uniform vec4 vertexColor;"
"varying vec4 brushColor;"

"void main(){"
	"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;"
	"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
	"gl_FrontColor = gl_Color;"

	"brushColor = vertexColor;"
"}";

const static std::string whiteboard_point_frag =
"uniform sampler2D tex0;"
"varying vec4 brushColor;"
"void main(){"
"vec4 color = texture2D(tex0, gl_TexCoord[0].st);"
"vec4 theBrushColor = brushColor;"
"theBrushColor.r *= brushColor.a * color.r;"
"theBrushColor.g *= brushColor.a * color.g;"
"theBrushColor.b *= brushColor.a * color.b;"
"theBrushColor *= color.a;"
"gl_FragColor = theBrushColor;"
//NEON EFFECTS!//"gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0/2.2));"
"}";

static std::string whiteboard_point_name = "whiteboard_point";


const std::string opacityFrag =
"uniform sampler2D tex0;\n"
"uniform float opaccy;\n"
"void main()\n"
"{\n"
"    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    color = texture2D( tex0, gl_TexCoord[0].st );\n"
"    color *= gl_Color;\n"
"    color *= opaccy;\n"
"    gl_FragColor = color;\n"
"}\n";

const std::string vertShader =
"void main()\n"
"{\n"
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
"  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"  gl_FrontColor = gl_Color;\n"
"}\n";

static std::string shaderNameOpaccy = "opaccy_shader";
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
	, mOutputShader(vertShader, opacityFrag, shaderNameOpaccy)
	, mCanvasFileLoaderClient(eng)
{
	mBlobType = BLOB_TYPE;
	mOutputShader.loadShaders();

	mPointShader.loadShaders();
	mFboGeneral = std::move(mEngine.getFbo());
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
			mSerializedPointsQueue.push_back( std::make_pair(localPoint.xy(), localPoint.xy()));
			renderLine(localPoint, localPoint);
			markAsDirty(sPointsQueueDirty);
		}
		if(ti.mPhase == ds::ui::TouchInfo::Moved){
			mSerializedPointsQueue.push_back( std::make_pair(prevPoint.xy(), localPoint.xy()) );
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
	if (imagePath.empty()){
		DS_LOG_WARNING( "No brush image path supplied to drawing canvas" );
		return;
	}
	setImageFile(imagePath);
	markAsDirty( sBrushImagePathDirty );
}

void DrawingCanvas::clearCanvas(){
	auto w = getWidth();
	auto h = getHeight();

	if(!mDrawTexture || mDrawTexture.getWidth() != w || mDrawTexture.getHeight() != h) {
		ci::gl::Texture::Format format;
		format.setTarget(GL_TEXTURE_2D);
		format.setMagFilter(GL_LINEAR);
		format.setMinFilter(GL_LINEAR);
		mDrawTexture = ci::gl::Texture((int)w, (int)h, format);
	}

	ds::gl::SaveCamera		save_camera;

	mFboGeneral->attach(mDrawTexture);
	mFboGeneral->begin();

	ci::Area fboBounds(0, 0, mFboGeneral->getWidth(), mFboGeneral->getHeight());
	ci::gl::setViewport(fboBounds);
	ci::CameraOrtho camera;
	camera.setOrtho(static_cast<float>(fboBounds.getX1()), static_cast<float>(fboBounds.getX2()), static_cast<float>(fboBounds.getY2()), static_cast<float>(fboBounds.getY1()), -1.0f, 1.0f);
	ci::gl::setMatrices(camera);

	ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	mPointShader.getShader().unbind();
	mFboGeneral->end();
	mFboGeneral->detach();

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
		mDrawTexture = *loaderTex;
		mCanvasFileLoaderClient.clear();
	}

	// If any serialized points have been received from the server, draw them
	while (!mSerializedPointsQueue.empty()) {
		auto points = mSerializedPointsQueue.front();
		renderLine( ci::Vec3f( points.first ), ci::Vec3f( points.second ) );
		mSerializedPointsQueue.pop_front();
	}

	if(!mDrawTexture) return;

	if(mDrawTexture) {
	//	if(getBlendMode() == ds::ui::BlendMode::NORMAL){
	//		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//}

		// ignore the "color" setting
		ci::gl::color(ci::Color::white());
		// The true flag is for premultiplied alpha, which this texture is
		ci::gl::enableAlphaBlending(true);
		ci::gl::GlslProg& shaderBase = mOutputShader.getShader();
		if(shaderBase) {
			shaderBase.bind();
			shaderBase.uniform("tex0", 0);
			shaderBase.uniform("opaccy", mDrawOpacity);
			mUniform.applyTo(shaderBase);
		}


		if(getPerspective()){
			ci::gl::draw(mDrawTexture, ci::Rectf(0.0f, 0.0f, static_cast<float>(mDrawTexture.getWidth()), static_cast<float>(mDrawTexture.getHeight())));
		} else {
			ci::gl::draw(mDrawTexture, ci::Rectf(0.0f, static_cast<float>(mDrawTexture.getHeight()), static_cast<float>(mDrawTexture.getWidth()), 0.0f));
		}

		if(shaderBase){
			shaderBase.unbind();
		}
	}
}

void DrawingCanvas::renderLine(const ci::Vec3f& start, const ci::Vec3f& end){
	auto brushTexture = getImageTexture();

	if(!brushTexture){
		DS_LOG_WARNING("No brush image texture when trying to render a line in Drawing Canvas");
		return;
	}
	
	float brushPixelStep = 3.0f;
	int vertexCount = 0;

	std::vector<ci::Vec2f> drawPoints;

	// Create a point for every pixel between start and end for smoothness
	int count = std::max<int>((int)ceilf(sqrtf((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y)) / (float)brushPixelStep), 1);
	for(int i = 0; i < count; ++i) {
		drawPoints.push_back(ci::Vec2f(start.x + (end.x - start.x) * ((float)i / (float)count), start.y + (end.y - start.y) * ((float)i / (float)count)));
	}

	int w = (int)floorf(getWidth());
	int h = (int)floorf(getHeight());

	if(!mDrawTexture || mDrawTexture.getWidth() != w || mDrawTexture.getHeight() != h) {
		ci::gl::Texture::Format format;
		format.setTarget(GL_TEXTURE_2D);
		format.setMagFilter(GL_LINEAR);
		format.setMinFilter(GL_LINEAR);
		mDrawTexture = ci::gl::Texture(w, h, format);
	}

	ds::gl::SaveCamera		save_camera;

	ci::gl::SaveFramebufferBinding bindingSaver;
	mFboGeneral->attach(mDrawTexture);
	mFboGeneral->begin();

	ci::Area fboBounds(0, 0, mFboGeneral->getWidth(), mFboGeneral->getHeight());
	ci::gl::setViewport(fboBounds);
	ci::CameraOrtho camera;
	camera.setOrtho(static_cast<float>(fboBounds.getX1()), static_cast<float>(fboBounds.getX2()), static_cast<float>(fboBounds.getY2()), static_cast<float>(fboBounds.getY1()), -1.0f, 1.0f);
	ci::gl::setMatrices(camera);


	mPointShader.getShader().bind();
	mPointShader.getShader().uniform("tex0", 10);
	mPointShader.getShader().uniform("vertexColor", mBrushColor);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);

	if(mEraseMode){
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	{
		ci::gl::SaveTextureBindState saveBindState(brushTexture->getTarget());
		ci::gl::BoolState saveEnabledState(brushTexture->getTarget());
		ci::gl::ClientBoolState vertexArrayState(GL_VERTEX_ARRAY);
		ci::gl::ClientBoolState texCoordArrayState(GL_TEXTURE_COORD_ARRAY);
		brushTexture->bind(10);

		glEnableClientState(GL_VERTEX_ARRAY);
		GLfloat verts[8];
		glVertexPointer(2, GL_FLOAT, 0, verts);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		GLfloat texCoords[8];
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

		auto srcArea = brushTexture->getCleanBounds();
		const ci::Rectf srcCoords = brushTexture->getAreaTexCoords(srcArea);

		float widdy = mBrushSize;
		float hiddy = mBrushSize / ((float)brushTexture->getWidth() / (float)brushTexture->getHeight());

		for(auto it : drawPoints){

			auto destRect = ci::Rectf(it.x - widdy / 2.0f, it.y - hiddy / 2.0f, it.x + widdy / 2.0f, it.y + hiddy / 2.0f);

			verts[0 * 2 + 0] = destRect.getX2(); verts[0 * 2 + 1] = destRect.getY1();
			verts[1 * 2 + 0] = destRect.getX1(); verts[1 * 2 + 1] = destRect.getY1();
			verts[2 * 2 + 0] = destRect.getX2(); verts[2 * 2 + 1] = destRect.getY2();
			verts[3 * 2 + 0] = destRect.getX1(); verts[3 * 2 + 1] = destRect.getY2();

			texCoords[0 * 2 + 0] = srcCoords.getX2(); texCoords[0 * 2 + 1] = srcCoords.getY1();
			texCoords[1 * 2 + 0] = srcCoords.getX1(); texCoords[1 * 2 + 1] = srcCoords.getY1();
			texCoords[2 * 2 + 0] = srcCoords.getX2(); texCoords[2 * 2 + 1] = srcCoords.getY2();
			texCoords[3 * 2 + 0] = srcCoords.getX1(); texCoords[3 * 2 + 1] = srcCoords.getY2();
			

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		brushTexture->unbind(10);
	}

	mDrawTexture.unbind();

	mPointShader.getShader().unbind();
	mFboGeneral->end();
	mFboGeneral->detach();
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
		buf.add<uint32_t>(mSerializedPointsQueue.size());
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
		ci::Vec2f p1, p2;
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
	if (!(mDrawTexture && mDrawTexture.getWidth() > 0 && mDrawTexture.getHeight() > 0 ))
		return;

	// This can't be done on the background thread because it needs
	// the main thread's GL context to get the texture data.
	ci::Surface8u surface(mDrawTexture);

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
