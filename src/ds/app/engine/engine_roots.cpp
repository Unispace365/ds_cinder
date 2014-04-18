#include "ds/app/engine/engine_roots.h"

#include "ds/app/engine/engine.h"
#include "ds/gl/save_camera.h"

namespace ds {

/**
 * \class ds::EngineRoot
 */
ds::ui::Sprite* EngineRoot::make(ui::SpriteEngine& e, const ds::sprite_id_t id, const bool perspective)	{
	return new ds::ui::Sprite(e, id, perspective);
}

EngineRoot::EngineRoot(const sprite_id_t id)
		: mSpriteId(id) {
}

EngineRoot::~EngineRoot() {
}

/**
 * \class ds::OrthRoot
 */
OrthRoot::OrthRoot(Engine& e, const sprite_id_t id)
		: inherited(id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSetViewport(true)
		, mSprite(EngineRoot::make(e, id, false)) {
}

void OrthRoot::setup(const Settings& s) {
	const bool			scaleWorldToFit = s.mDebugSettings.getBool("scale_world_to_fit", 0, false);
	const float			window_scale = s.mDebugSettings.getFloat("window_scale", 0, s.mDefaultScale);

	mSprite->setSize(s.mScreenRect.getWidth(), s.mScreenRect.getHeight());
	if (scaleWorldToFit) {
		mSprite->setScale(mEngine.getWidth()/mEngine.getWorldWidth(), mEngine.getHeight()/mEngine.getWorldHeight());
	} else if (window_scale != s.mDefaultScale) {
		mSprite->setScale(window_scale, window_scale);
	}
}

ds::ui::Sprite* OrthRoot::getSprite() {
	return mSprite.get();
}

void OrthRoot::clearChildren() {
	mSprite->clearChildren();
}

void OrthRoot::updateClient(const ds::UpdateParams& p) {
	mSprite->updateClient(p);
}

void OrthRoot::updateServer(const ds::UpdateParams& p) {
	mSprite->updateServer(p);
}

void OrthRoot::drawClient(const DrawParams& p) {
	if (mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	mSprite->drawClient(ci::gl::getModelView(), p);
}

void OrthRoot::drawServer(const DrawParams& p) {
	setGlCamera();
	mSprite->drawServer(ci::gl::getModelView(), p);
}

ui::Sprite* OrthRoot::getHit(const ci::Vec3f& point) {
	return mSprite->getHit(point);
}

void OrthRoot::setCinderCamera() {
	mCameraDirty = false;
	const ci::Rectf&		screen_rect(mEngine.getScreenRect());

	// I think this should be in setGlCamera, but keeping it compatible for now.
	if (mSetViewport) {
		ci::gl::setViewport(Area((int)screen_rect.getX1(), (int)screen_rect.getY2(), (int)screen_rect.getX2(), (int)screen_rect.getY1()));
	}
	mCamera.setOrtho(screen_rect.getX1(), screen_rect.getX2(), screen_rect.getY2(), screen_rect.getY1(), -1, 1);
	//gl::setMatrices(mCamera);
}

void OrthRoot::setViewport(const bool b) {
	mSetViewport = b;
}

void OrthRoot::markCameraDirty() {
	mCameraDirty = true;
}

void OrthRoot::setGlCamera() {
	if (mSetViewport) {
		const ci::Rectf&		screen_rect(mEngine.getScreenRect());
		ci::gl::setViewport(Area((int)screen_rect.getX1(), (int)screen_rect.getY2(), (int)screen_rect.getX2(), (int)screen_rect.getY1()));
	}
	ci::gl::setMatrices(mCamera);
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
}

/**
 * \class ds::PerspRoot
 */
PerspRoot::PerspRoot(Engine& e, const sprite_id_t id, const PerspCameraParams& p)
		: inherited(id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSprite(EngineRoot::make(e, id, true)) {
	mCamera.setEyePoint(p.mPosition);
	mCamera.setCenterOfInterestPoint(p.mTarget);
	mCamera.setFov(p.mFov);
	mCamera.setNearClip(p.mNearPlane);
	mCamera.setFarClip(p.mFarPlane);
}

void PerspRoot::setup(const Settings& s) {
	mSprite->setSize(s.mScreenRect.getWidth(), s.mScreenRect.getHeight());
	mSprite->setDrawSorted(true);
}

ds::ui::Sprite* PerspRoot::getSprite() {
	return mSprite.get();
}

void PerspRoot::clearChildren() {
	mSprite->clearChildren();
}

void PerspRoot::updateClient(const ds::UpdateParams& p) {
	mSprite->updateClient(p);
}

void PerspRoot::updateServer(const ds::UpdateParams& p) {
	mSprite->updateServer(p);
}

void PerspRoot::drawClient(const DrawParams& p) {
	// XXX This shouldn't be necessary - the OrthoRoot should be setting
	// all camera properties whenever it draws. But for some reason, without
	// this, no later ortho roots will draw (but then it's fine once you go
	// back to the first root). This inspite of the fact that the ortho
	// setGlCamera code calls everything this does.
	gl::SaveCamera		save_camera;

	if (mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	glClear(GL_DEPTH_BUFFER_BIT);
	mSprite->drawClient(ci::gl::getModelView(), p);
}

void PerspRoot::drawServer(const DrawParams& p) {
	setGlCamera();
	// Redirect to client draw for now
	mSprite->drawClient(ci::gl::getModelView(), p);
}

ui::Sprite* PerspRoot::getHit(const ci::Vec3f& point) {
	ds::CameraPick			pick(mCamera, point, mSprite->getWidth(), mSprite->getHeight());
	return mSprite->getPerspectiveHit(pick);
}

PerspCameraParams PerspRoot::getCamera() const {
	PerspCameraParams		p;
	p.mPosition = mCamera.getEyePoint();
	p.mTarget = mCamera.getCenterOfInterestPoint();
	p.mFov = mCamera.getFov();
	p.mNearPlane = mCamera.getNearClip();
	p.mFarPlane = mCamera.getFarClip();
	return p;
}

void PerspRoot::setCamera(const PerspCameraParams& p) {
	if (p == getCamera()) return;

	mCamera.setEyePoint(p.mPosition);
	mCamera.setCenterOfInterestPoint(p.mTarget);
	mCamera.setPerspective(p.mFov, getWindowAspectRatio(), p.mNearPlane, p.mFarPlane);
	mCameraDirty = true;
}

void PerspRoot::markCameraDirty() {
	mCameraDirty = true;
}

void PerspRoot::setCinderCamera() {
	mCameraDirty = false;
//	mCamera.setEyePoint( Vec3f(0.0f, 0.0f, 100.0f) );
//	mCamera.setCenterOfInterestPoint( Vec3f(0.0f, 0.0f, 0.0f) );
	mCamera.setPerspective(mCamera.getFov(), getWindowAspectRatio(), mCamera.getNearClip(), mCamera.getFarClip());
}

void PerspRoot::setGlCamera() {
	ci::gl::setMatrices(mCamera);
	// enable the depth buffer (after all, we are doing 3D)
	//gl::enableDepthRead();
	//gl::enableDepthWrite();
	//gl::translate(-getWorldWidth()/2.0f, -getWorldHeight()/2.0f, 0.0f);
}

} // namespace ds
