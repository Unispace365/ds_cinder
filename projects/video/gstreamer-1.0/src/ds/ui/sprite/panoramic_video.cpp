#include "stdafx.h"

#include "panoramic_video.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/blob_registry.h>
#include <ds/app/blob_reader.h>
#include <ds/data/data_buffer.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/app.h>


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
	, mInvertX(false)
	, mInvertY(true)
	, mXSensitivity(0.15f)
	, mYSensitivity(0.15f)
	, mFov(60.0f)
	, mAutoSync(true)
	, mPanning(0.0f)
	, mXRot(0.0f)
	, mYRot(-90.0f)
{
	mBlobType = _BLOB;
	setUseShaderTexture(true);
	setTransparent(true);

	mSphereVbo = ci::gl::VboMesh::create(ci::geom::Sphere().subdivisions(120).radius(200.0f));

	resetCamera();
	
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite*, const ds::ui::TouchInfo& ti){
		handleDrag(ci::vec2(ti.mDeltaPoint));		
	});
}

void PanoramicVideo::handleDrag(const ci::vec2& swipe)
{
	mXRot += swipe.y * mYSensitivity;
	mYRot += swipe.x * mXSensitivity;
	if(mXRot > 90.0f){ mXRot = 90.0f; }
	if(mXRot < -90.0f){ mXRot = -90.0f; }
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
		buf.add(mXRot);
		buf.add(mYRot);
	}
}

void PanoramicVideo::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == mSphereCoordsAtt) {
		mXRot = buf.read<float>();
		mYRot = buf.read<float>();

	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

// no need to build the base render batch
void PanoramicVideo::onBuildRenderBatch(){

}

void PanoramicVideo::drawLocalClient(){

	if(mVideoSprite){

		ci::gl::TextureRef videoTexture = mVideoSprite->getFinalOutTexture();
		if(!videoTexture) return;
		
		ci::Area newViewportBounds;

		DS_REPORT_GL_ERRORS();

		if(!getPerspective()){
			ci::Rectf bb = getBoundingBox();
			ci::vec3 ul = getParent()->localToGlobal(ci::vec3(bb.getUpperLeft(), 0.0f));
			ci::vec3 br = getParent()->localToGlobal(ci::vec3(bb.getLowerRight(), 0.0f));

			float yScale = mEngine.getSrcRect().getHeight() / mEngine.getDstRect().getHeight();
			float xScale = mEngine.getSrcRect().getWidth() / mEngine.getDstRect().getWidth();

			// even though we're not in perspective, the cinder perspective camera starts from the bottom of the window
			// and counts upwards for y. So reverse that to draw where we expect
			ul = ul - ci::vec3(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			ul.y /= yScale;
			ul.x /= xScale;
			ul.y = ci::app::getWindowBounds().getHeight() - ul.y;

			br = br - ci::vec3(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			br.y /= yScale;
			br.x /= xScale;
			br.y = ci::app::getWindowBounds().getHeight() - br.y;

			newViewportBounds = ci::Area(ci::vec2(ul), ci::vec2(br));
		} else {
			ci::vec3 ul = getParent()->localToGlobal(getPosition() - getCenter()*getSize());
			ul = ul - ci::vec3(mEngine.getSrcRect().getUpperLeft(), 0.0f);
			ci::vec3 br = ul + ci::vec3(getWidth(), getHeight(), 0.0f);
			newViewportBounds = ci::Area(ci::vec2(ul.x, br.y), ci::vec2(br.x, ul.y));
		}

		// might have to figure this out for client/server
		ci::gl::ScopedViewport newViewport(newViewportBounds.getX1(), newViewportBounds.getY1(), newViewportBounds.getWidth(), newViewportBounds.getHeight());

		videoTexture->bind();

		ci::gl::ScopedMatrices matricies;
		ci::gl::setMatrices(mCamera);
		
		ci::gl::rotate(ci::toRadians(mXRot), 0.0f, 0.0f, 1.0f);
		ci::gl::rotate(ci::toRadians(mYRot), 0.0f, 1.0f, 0.0);
		ci::gl::draw(mSphereVbo);

		videoTexture->unbind();
		
	}
}

void PanoramicVideo::resetCamera(){
	mCamera.setPerspective(mFov, getWidth() / getHeight(), 0.1f, 5000.0f);
	mCamera.setWorldUp(ci::vec3(0.0f, -1.0f, 0.0f));
	mCamera.setEyePoint(ci::vec3());
	mCamera.setPivotDistance(200.0f);
	mXRot = 0.0f;
	mYRot = -90.0f;
	mCamera.lookAt(ci::vec3(200.0f, 0.0f, 0.0f));

}

void PanoramicVideo::onSizeChanged() {
	mCamera.setPerspective(mFov, getWidth() / getHeight(), 0.1f, 5000.0f);
}

void PanoramicVideo::loadVideo(const std::string& path) {
	setResource(ds::Resource(path));
}

void PanoramicVideo::setResource(const ds::Resource& resource) {

	if(mVideoSprite){
		mVideoSprite->release();
		mVideoSprite = nullptr;
	}

	// this does some dumb shit
	// any added children are listened to and linked to the video sprite
	// this is for the net sync stuff
	// there should be a better way to do this
	auto video_sprite = addChildPtr(new ds::ui::Video(mEngine));

	//Need to enable this to enable panning 
	video_sprite->generateAudioBuffer(true);
	video_sprite->setPan(mPanning);
	video_sprite->setPlayableInstances(mPlayableInstances);
	video_sprite->setAutoStart(true);
	video_sprite->setLooping(true);
	video_sprite->setAutoSynchronize(mAutoSync);
	video_sprite->setResource(resource);
	video_sprite->setFinalRenderToTexture(true);

	resetCamera();
}

ds::ui::Video* PanoramicVideo::getVideo() const {
	return mVideoSprite;
}

void PanoramicVideo::setFOV(const float fov){
	mFov = fov;
	resetCamera();
}

void PanoramicVideo::setPan(const float audioPan){
	mPanning = audioPan;
	if(mVideoSprite){
		mVideoSprite->setPan(mPanning);
	}
}

void PanoramicVideo::setPlayableInstances(const std::vector<std::string> instances){
	mPlayableInstances = instances;
	if(mVideoSprite){
		mVideoSprite->setPlayableInstances(instances);
	}

}

void PanoramicVideo::setAutoSyncronize(const bool doSync){
	mAutoSync = doSync;
	if(mVideoSprite){
		mVideoSprite->setAutoSynchronize(mAutoSync);
	}
}

}} //!ds::ui
