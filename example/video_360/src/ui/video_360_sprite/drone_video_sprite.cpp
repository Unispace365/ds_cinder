#include "drone_video_sprite.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/blob_registry.h>
#include <ds/app/blob_reader.h>
#include <ds/data/data_buffer.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/video.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/app.h>

#include <cinder/DataSource.h>
#include <cinder/Tween.h>
#include "cinder/ImageIo.h"


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
				ds::ui::DroneVideoSprite::installSprite(engine);
			});
		}
	} INIT;
static char _BLOB;
} //!anonymous namespace

namespace ds {
	namespace ui {


void DroneVideoSprite::installAsServer(ds::BlobRegistry& registry)
{
	_BLOB = registry.add([](ds::BlobReader& r)
	{
		Sprite::handleBlobFromClient(r);
	});
}

void DroneVideoSprite::installAsClient(ds::BlobRegistry& registry)
{
	_BLOB = registry.add([](ds::BlobReader& r)
	{
		Sprite::handleBlobFromServer<DroneVideoSprite>(r);
	});
}

void DroneVideoSprite::installSprite(ds::Engine& engine)
{
	engine.installSprite(installAsServer, installAsClient);
}

DroneVideoSprite::DroneVideoSprite(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mVideoSprite(nullptr)
	, mVideoTexture(nullptr)
	, mSphere(ci::Sphere(ci::Vec3f::zero(), 100.0f)) // doesn't really matter how big this is. FIXME
	, util::Configurable(mEngine, "drone")
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

void DroneVideoSprite::handleDrag(const ci::Vec2f& swipe)
{
	auto invert_x = getSettings().getBool("x_invert") ? -1.0f : 1.0f;
	auto invert_y = getSettings().getBool("y_invert") ? -1.0f : 1.0f;
	auto new_theta = getTheta() - invert_y * (swipe.y / getSettings().getFloat("y_factor"));
	if (new_theta < 0.1f) new_theta = 0.1f;
	else if (new_theta > 180.0f) new_theta = 180.0f - 0.1f;
	setRotation(new_theta, getPhi() - invert_x * (swipe.x / getSettings().getFloat("x_factor")), getRotation().z);
}

void DroneVideoSprite::onRotationChanged()
{
	setSphericalCoord(getRotation().x, getRotation().y);
}

void DroneVideoSprite::onChildAdded(ds::ui::Sprite& child)
{
	if (mVideoSprite)
	{
		DS_LOG_WARNING("Multiple video sprites were attempted to be registered. Gonna ignore it.");
		return;
	}
	else if (auto video = dynamic_cast<ds::ui::Video*>(&child))
	{
		mVideoSprite = video;
		mVideoSprite->setTransparent(false);
		setTransparent(false);

		mVideoSprite->setFinalRenderToTexture(true);
		// register the texture
		mVideoTexture = mVideoSprite->getFinalOutTexture();
	}
}

void DroneVideoSprite::onChildRemoved(ds::ui::Sprite& child)
{
	if (mVideoSprite)
	{
		if (auto video = dynamic_cast<ds::ui::Video*>(&child))
		{
			if (video == mVideoSprite)
			{
				mVideoSprite = nullptr;
			}
		}
	}
}

void DroneVideoSprite::updateServer(const ds::UpdateParams& up)
{
	ds::ui::Sprite::updateServer(up);

	if (mVideoSprite && !mVideoTexture) {
		mVideoTexture = mVideoSprite->getFinalOutTexture();
	}
}

void DroneVideoSprite::updateClient(const ds::UpdateParams& up)
{
	ds::ui::Sprite::updateClient(up);
	if (mVideoSprite && !mVideoTexture) {
		mVideoTexture = mVideoSprite->getFinalOutTexture();
	}
}

void DroneVideoSprite::drawLocalClient()
{
	if (mVideoTexture && mVideoSprite && mSpriteShader.getName().compare(shader_name) == 0)
	{
		mVideoTexture = mVideoSprite->getFinalOutTexture();
		mVideoTexture->enableAndBind();
		ci::gl::GlslProg& shaderBase = mSpriteShader.getShader();
		shaderBase.bind();

		shaderBase.uniform("tex0", static_cast<int>(mVideoTexture->getTarget()));
		shaderBase.uniform("mv", mCamera.getModelViewMatrix());
		shaderBase.uniform("p", mCamera.getProjectionMatrix());
		shaderBase.uniform("radius", mSphere.getRadius());

		ci::gl::draw(mSphere, 120);

		mVideoTexture->unbind();
		shaderBase.unbind();
	}
}

void DroneVideoSprite::resetCamera()
{
	mCamera.setPerspective(60.0f, getWidth() / getHeight(), 0.1f, 1000.0f);
	mCamera.setWorldUp(ci::Vec3f::zAxis());
	mCamera.setEyePoint(mSphere.getCenter());
	mCamera.setCenterOfInterest(mSphere.getRadius());
	lookFront();
}

void DroneVideoSprite::lookFront()
{
	setSphericalCoord(90.0f, 0);
}

void DroneVideoSprite::lookBack()
{
	setSphericalCoord(-90.0f, 0);
}

void DroneVideoSprite::lookUp()
{
	setSphericalCoord(0, 0);
}

void DroneVideoSprite::lookDown()
{
	setSphericalCoord(180.0f, 0);
}

void DroneVideoSprite::lookRight()
{
	setSphericalCoord(90.0f, -90.0f);
}

void DroneVideoSprite::lookLeft()
{
	setSphericalCoord(90.0f, 90.0f);
}

namespace {
void cleanAngle(float& degree) {
	degree = ci::math<float>::fmod(degree, 360.0f);
	if (degree < 0) degree += 360.0f;
	if (degree < 0.0001f) degree = 0;
}}

void DroneVideoSprite::setSphericalCoord(float theta, float phi)
{
	// Clean for (over/under)flows and negative angles
	cleanAngle(phi);
	cleanAngle(theta);

	// Cache for later use
	mSphericalAngles.x = theta;
	mSphericalAngles.y = phi;

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

void DroneVideoSprite::onSizeChanged()
{
	if (mVideoSprite) {
		mVideoSprite->setSize(getWidth(), getHeight());
	}
	resetCamera();
}

void DroneVideoSprite::installVideo(const std::string& path)
{
	auto video_sprite = addChildPtr(new ds::ui::Video(mEngine));
	video_sprite->setAutoStart(true);
	video_sprite->setLooping(true);
	video_sprite->loadVideo(path);
	//video_sprite->setSize(getWidth(), getHeight());
}

void DroneVideoSprite::installVideo( ds::ui::Video* const video, const std::string& path)
{
	ds::ui::Video* const video_sprite = addChildPtr(video);
	video_sprite->setAutoStart(true);
	video_sprite->setLooping(true);
	video_sprite->loadVideo(path);
	video_sprite->setSize(getWidth(), getHeight());

//	video_sprite->setSize(getScaleWidth(), getScaleHeight());
	//video_sprite->setScale(5,5);
	setSize(video_sprite->getWidth(), video_sprite->getHeight());
	//setSize(10, 10);
}

ds::ui::Video* DroneVideoSprite::getVideo() const
{
	return mVideoSprite;
}

}} //!dlpr::view
