#include "drawing_canvas.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/file_meta_data.h>

#include <ds/gl/save_camera.h>
#include <cinder/ImageIo.h>

#include <cinder/Rand.h>

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
"brushColor.r *= color.a * brushColor.a;"
"brushColor.g *= color.a * brushColor.a;"
"brushColor.b *= color.a * brushColor.a;"
"gl_FragColor = brushColor * color;"
"}";

static std::string whiteboard_point_name = "whiteboard_point";

}

namespace ds {
namespace ui {

DrawingCanvas::DrawingCanvas(ds::ui::SpriteEngine& eng, const std::string& brushImagePath)
	: ds::ui::Sprite(eng)
	, mBrushSize(24.0f)
	, mBrushColor(1.0f, 0.0f, 0.0f, 0.5f)
	, mPointShader(whiteboard_point_vert, whiteboard_point_frag, whiteboard_point_name)
	, mBrushImage(nullptr)
{
	mPointShader.loadShaders();
	mFboGeneral = std::move(mEngine.getFbo());
	DS_REPORT_GL_ERRORS();

	setBrushImage(brushImagePath);

	setBrushColor(ci::ColorA(1.0f, 0.3f, 0.3f, 0.7f));
	setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	setTransparent(false);
	setColor(ci::Color(1.0f, 1.0f, 1.0f));
	setUseShaderTexture(true);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti){
		auto localPoint = globalToLocal(ti.mCurrentGlobalPoint);
		if(ti.mPhase == ds::ui::TouchInfo::Added){
			renderLine(localPoint, localPoint);
		}
		if(ti.mPhase == ds::ui::TouchInfo::Moved){ 
			renderLine(localPoint - ti.mDeltaPoint, localPoint);
		}
	});
}

void DrawingCanvas::setBrushColor(const ci::ColorA& brushColor){
	mBrushColor = brushColor;
}

void DrawingCanvas::setBrushColor(const ci::Color& brushColor){
	mBrushColor.r = brushColor.r; mBrushColor.g = brushColor.g; mBrushColor.b = brushColor.b;
}

void DrawingCanvas::setBrushOpacity(const float brushOpacity){
	mBrushColor.a = brushOpacity;
}

const ci::ColorA& DrawingCanvas::getBrushColor(){
	return mBrushColor;
}

void DrawingCanvas::setBrushSize(const float brushSize){
	mBrushSize = brushSize;
}

const float DrawingCanvas::getBrushSize(){
	return mBrushSize;
}

void DrawingCanvas::setBrushImage(const std::string& imagePath){
	if(mBrushImage){
		mBrushImage->release();
		mBrushImage = nullptr;
	}

	if(imagePath.empty()){
		DS_LOG_WARNING("No brush image path supplied to drawing canvas");
		return;
	}

	auto expandedPath = ds::Environment::expand(imagePath);

	if(!ds::safeFileExistsCheck(expandedPath, false)){
		DS_LOG_WARNING("No brush image path supplied to drawing canvas");
		return;
	}
	
	mBrushImage = new ds::ui::Image(mEngine, expandedPath);
	addChildPtr(mBrushImage);
	mBrushImage->hide();
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
}

void DrawingCanvas::drawLocalClient(){
	if(!mDrawTexture) return;

	if(mDrawTexture) {
		if(getBlendMode() == ds::ui::BlendMode::NORMAL){
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		if(getPerspective()){
			ci::gl::draw(mDrawTexture, ci::Rectf(0.0f, 0.0f, static_cast<float>(mDrawTexture.getWidth()), static_cast<float>(mDrawTexture.getHeight())));
		} else {
			ci::gl::draw(mDrawTexture, ci::Rectf(0.0f, static_cast<float>(mDrawTexture.getHeight()), static_cast<float>(mDrawTexture.getWidth()), 0.0f));
		}
	}
}

void DrawingCanvas::renderLine(const ci::Vec3f& start, const ci::Vec3f& end){
	if(!mBrushImage){
		DS_LOG_WARNING("No brush image when trying to render a line in Drawing Canvas");
		return;
	}

	auto brushTexture = mBrushImage->getImageTexture();

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


	mPointShader.getShader().bind();
	mPointShader.getShader().uniform("tex0", 0);
	mPointShader.getShader().uniform("vertexColor", mBrushColor);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	{
		ci::gl::SaveTextureBindState saveBindState(brushTexture->getTarget());
		ci::gl::BoolState saveEnabledState(brushTexture->getTarget());
		ci::gl::ClientBoolState vertexArrayState(GL_VERTEX_ARRAY);
		ci::gl::ClientBoolState texCoordArrayState(GL_TEXTURE_COORD_ARRAY);
		brushTexture->bind(0);

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

		brushTexture->unbind(0);
	}

	mPointShader.getShader().unbind();
	mFboGeneral->end();
	mFboGeneral->detach();
}

} // namespace ui
} // namespace ds
