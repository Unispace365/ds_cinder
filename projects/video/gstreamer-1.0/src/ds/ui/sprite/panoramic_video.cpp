#include "panoramic_video.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/blob_registry.h>
#include <ds/app/blob_reader.h>
#include <ds/data/data_buffer.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/app.h>


static std::string shader_name("drone");

static std::string drone_vert =
"/////////////////////////////////////////////////////////////////////// \n"
"// Vertex shader inputs (You will also need to set tex0 for frag shader \n"
"/////////////////////////////////////////////////////////////////////// \n"
"																		 \n"
"// ViewModel and Projection											 \n"
"// NOTE: order matters!												 \n"
"// Example for mv: ci::MayaCamUI::getCamera::getModelViewMatrix()		 \n"
"// Example for p:  ci::MayaCamUI::getCamera::getProjectionMatrix()		 \n"
"uniform mat4 mv, p;													 \n"
"// globe / sphere radius (passed to ci::gl::drawSphere for example)	 \n"
"uniform float radius;													 \n"
"																		 \n"
"/////////////////////////////////////////////////////////////////////// \n"
"// Vertex shader outputs												 \n"
"/////////////////////////////////////////////////////////////////////// \n"
"																		 \n"
"// calculated here and passed to fragment shader						 \n"
"varying vec4 texCoords;												 \n"
"																		 \n"
"/////////////////////////////////////////////////////////////////////// \n"
"// Vertex shader main() vertex to world mapping						 \n"
"/////////////////////////////////////////////////////////////////////// \n"
"																		 \n"
"void main(void) {														 \n"
"	// build the model view projection									 \n"
"	mat4 mvp = p*mv;													 \n"
"	// get the current world position									 \n"
"	gl_Position = mvp * gl_Vertex;										 \n"
"	// figure out where in the texture we are standing					 \n"
"	texCoords = gl_Vertex * (1.0 / radius);								 \n"
"}																		 \n"
;

static std::string drone_frag =
"//////////////////////////////////////////////////////////////////////// \n"
"// Fragment shader inputs (from C++)									  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"																		  \n"
"// Cinder example: ci::gl::Texture::getId()							  \n"
"uniform sampler2D tex0;												  \n"
"																		  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"// Fragment shader inputs (from Vertex shader)							  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"																		  \n"
"varying vec4 texCoords;												  \n"
"																		  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"// Fragment shader defines												  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"																		  \n"
"#define PI 3.1415926													  \n"
"																		  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"// Fragment shader texturing main()									  \n"
"//////////////////////////////////////////////////////////////////////// \n"
"																		  \n"
"void main(void) {														  \n"
"	vec2 longLat = vec2(												  \n"
"		1.0 - (atan(texCoords.y, texCoords.x) / PI + 1.0) * 0.5,		  \n"
"		1.0 - (asin(texCoords.z) / PI + 0.5));							  \n"
"																		  \n"
"	gl_FragColor = texture2D(tex0, longLat);							  \n"
"}																		  \n"
;



namespace {
	static struct Initializer {
		Initializer() {
			ds::App::AddStartup([](ds::Engine& engine) {
				ds::ui::PanoramicVideo::installSprite(engine);
			});
		}
	} INIT;
	static char _BLOB;
} //!anonymous namespace

namespace ds {
namespace ui {

	const char mSphereCoordsAtt = 81;
	const DirtyState&	SPHERE_DIRTY = INTERNAL_A_DIRTY;

void PanoramicVideo::installAsServer(ds::BlobRegistry& registry)
{
	_BLOB = registry.add([](ds::BlobReader& r)
	{
		Sprite::handleBlobFromClient(r);
	});
}

void PanoramicVideo::installAsClient(ds::BlobRegistry& registry)
{
	_BLOB = registry.add([](ds::BlobReader& r)
	{
		Sprite::handleBlobFromServer<PanoramicVideo>(r);
	});
}

void PanoramicVideo::installSprite(ds::Engine& engine)
{
	engine.installSprite(installAsServer, installAsClient);
}

PanoramicVideo::PanoramicVideo(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mVideoSprite(nullptr)
	, mVideoTexture(nullptr)
	, mSphere(ci::Sphere(ci::Vec3f::zero(), 100.0f)) // doesn't really matter how big this is. FIXME
	, mInvertX(false)
	, mInvertY(true)
	, mXSensitivity(5.0f)
	, mYSensitivity(5.0f)
	, mFov(60.0f)
{
	mBlobType = _BLOB;
	setUseShaderTextuer(true);
	setTransparent(true);
	addNewMemoryShader(drone_vert, drone_frag, shader_name);

	resetCamera();
	
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti){
		handleDrag(ti.mDeltaPoint.xy());
	});
}

void PanoramicVideo::handleDrag(const ci::Vec2f& swipe)
{
	auto invert_x = mInvertX ? 1.0f : -1.0f;
	auto invert_y = mInvertY ? 1.0f : -1.0f;
	auto new_theta = getTheta() - invert_y * (swipe.y / mYSensitivity);
	if (new_theta < 0.1f) new_theta = 0.1f;
	else if (new_theta > 180.0f) new_theta = 180.0f - 0.1f;
	setSphericalCoord(new_theta, getPhi() - invert_x * (swipe.x / mXSensitivity));
}

void PanoramicVideo::setDragParams(const float xSensitivity, const float ySensitivity){
	if(xSensitivity != 0.0f){
		mXSensitivity = xSensitivity;
	}

	if(ySensitivity != 0.0f){
		mYSensitivity = ySensitivity;
	}
}

void PanoramicVideo::setDragInvert(const bool xInvert, const bool yInvert){
	mInvertX = xInvert;
	mInvertY = yInvert;
}

void PanoramicVideo::onChildAdded(ds::ui::Sprite& child){
	if (mVideoSprite){
		DS_LOG_WARNING("Multiple video sprites were attempted to be registered. Gonna ignore it.");
		return;
	} else if(auto video = dynamic_cast<ds::ui::Video*>(&child)){
		mVideoSprite = video;
		mVideoSprite->setTransparent(false);
		setTransparent(false);

		mVideoSprite->setFinalRenderToTexture(true);
		// register the texture
		mVideoTexture = mVideoSprite->getFinalOutTexture();
	}
}

void PanoramicVideo::onChildRemoved(ds::ui::Sprite& child){
	if (mVideoSprite == &child){
		mVideoSprite = nullptr;
	}
}

void PanoramicVideo::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if(mDirty.has(SPHERE_DIRTY)) {
		buf.add(mSphereCoordsAtt);
		buf.add(mSphericalAngles.x);
		buf.add(mSphericalAngles.y);
	}
}

void PanoramicVideo::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == mSphereCoordsAtt) {
		float theta = buf.read<float>();
		float phi = buf.read<float>();
		setSphericalCoord(theta, phi);

	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

void PanoramicVideo::drawLocalClient(){

	if (mVideoSprite && mSpriteShader.getName().compare(shader_name) == 0){

		mVideoTexture = mVideoSprite->getFinalOutTexture();
		if(!mVideoTexture) return;
		//save off original viewport - restore after we are done
		ci::Area viewport = ci::gl::getViewport();
		DS_REPORT_GL_ERRORS();


		if(!getPerspective()) {
			ci::Rectf bb = getBoundingBox();
			ci::Vec3f ul = getParent()->localToGlobal(ci::Vec3f(bb.getUpperLeft(), 0.0f));
			ci::Vec3f br = getParent()->localToGlobal(ci::Vec3f(bb.getLowerRight(), 0.0f));

			float yScale = mEngine.getSrcRect().getHeight() / mEngine.getDstRect().getHeight();
			float xScale = mEngine.getSrcRect().getWidth() / mEngine.getDstRect().getWidth();

			// even though we're not in perspective, the cinder perspective camera starts from the bottom of the window
			// and counts upwards for y. So reverse that to draw where we expect
			ul = ul - ci::Vec3f(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			ul.y /= yScale;
			ul.x /= xScale;
			ul.y = ci::app::getWindowBounds().getHeight() - ul.y;

			br = br - ci::Vec3f(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			br.y /= yScale;
			br.x /= xScale;
			br.y = ci::app::getWindowBounds().getHeight() - br.y;

			ci::gl::setViewport(ci::Area(ul.xy(), br.xy()));
		} else {
			ci::Vec3f ul = getParent()->localToGlobal(ci::Vec3f(getPosition().xy() - getCenter().xy()*getSize().xy(), 0.0f));
			ul = ul - ci::Vec3f(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			ci::Vec3f br = ul + ci::Vec3f(getWidth(), getHeight(), 0.0f);
			ci::gl::setViewport(ci::Area(ci::Vec2f(ul.x,br.y), ci::Vec2f(br.x, ul.y)));
		}

		mVideoTexture->enableAndBind();
		ci::gl::GlslProg& shaderBase = mSpriteShader.getShader();
		shaderBase.bind();

		shaderBase.uniform("tex0", 0);// static_cast<int>(mVideoTexture->get()));
		shaderBase.uniform("mv", mCamera.getModelViewMatrix());
		shaderBase.uniform("p", mCamera.getProjectionMatrix());
		shaderBase.uniform("radius", mSphere.getRadius());

		ci::gl::draw(mSphere, 12);

		mVideoTexture->unbind();
		shaderBase.unbind();

		ci::gl::setViewport(viewport);
	}
}

void PanoramicVideo::resetCamera(){
	mCamera.setPerspective(mFov, getWidth() / getHeight(), 0.1f, 5000.0f);
	mCamera.setWorldUp(ci::Vec3f::zAxis());
	mCamera.setEyePoint(mSphere.getCenter());
	mCamera.setCenterOfInterest(mSphere.getRadius());
	lookFront();
}

namespace {
void cleanAngle(float& degree) {
	degree = ci::math<float>::fmod(degree, 360.0f);
	if (degree < 0) degree += 360.0f;
	if (degree < 0.0001f) degree = 0;
}}

void PanoramicVideo::setSphericalCoord(float theta, float phi){
	// Clean for (over/under)flows and negative angles
	cleanAngle(phi);
	cleanAngle(theta);

	// Cache for later use
	mSphericalAngles.x = theta;
	mSphericalAngles.y = phi;

	markAsDirty(SPHERE_DIRTY);

	// Convert to radian
	theta = ci::toRadians(getTheta());
	phi = ci::toRadians(getPhi());

	// Calculate target point
	ci::Vec3f target;
	target.x = mSphere.getRadius() * ci::math<float>::sin(theta) * ci::math<float>::cos(phi);
	target.y = mSphere.getRadius() * ci::math<float>::sin(theta) * ci::math<float>::sin(phi);
	target.z = mSphere.getRadius() * ci::math<float>::cos(theta);

	mCamera.lookAt(target);
}

void PanoramicVideo::onSizeChanged(){
	resetCamera();
}

void PanoramicVideo::loadVideo(const std::string& path){
	if(mVideoSprite){
		mVideoSprite->release();
		mVideoSprite = nullptr;
	}

	mVideoTexture = nullptr;

	auto video_sprite = addChildPtr(new ds::ui::Video(mEngine));

	//Need to enable this to enable panning 
	video_sprite->generateAudioBuffer(true);
	video_sprite->setAutoStart(true);
	video_sprite->setLooping(true);
	video_sprite->loadVideo(path);

	resetCamera();
}

ds::ui::Video* PanoramicVideo::getVideo() const {
	return mVideoSprite;
}

void PanoramicVideo::setFOV(const float fov){
	mFov = fov;
	resetCamera();
}

}} //!ds::ui
