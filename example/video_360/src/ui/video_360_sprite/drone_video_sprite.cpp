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

namespace {
	static struct Initializer {
		Initializer() {
			ds::App::AddStartup([](ds::Engine& engine) {
				dlpr::view::DroneVideoSprite::installSprite(engine);
			});
		}
	} INIT;
static char _BLOB;
} //!anonymous namespace

namespace dlpr {
namespace view {

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
	try {
		std::string name("video_360");
		addNewBaseShader(std::pair<std::string, std::string>
			(ds::Environment::expand("%APP%/data/shaders"),
			"globe")
			);

	}
	catch (ci::gl::GlslProgCompileExc &exc) {
		DS_LOG_WARNING( "Shader compile error: " << exc.what() );
	}
	catch (...) {
		DS_LOG_WARNING("Unable to load shader");
	}
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
	if (mVideoTexture && mVideoSprite && mSpriteShader.getName().compare("globe") == 0)
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
	video_sprite->setSize(getWidth(), getHeight());
}

void DroneVideoSprite::installVideo( ds::ui::Video* const video, const std::string& path)
{
	ds::ui::Video* const video_sprite = addChildPtr(video);
	video_sprite->setAutoStart(true);
	video_sprite->setLooping(true);
	video_sprite->loadVideo(path);
	video_sprite->setSize(getWidth(), getHeight());
}

ds::ui::Video* DroneVideoSprite::getVideo() const
{
	return mVideoSprite;
}

}} //!dlpr::view
