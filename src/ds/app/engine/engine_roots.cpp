#include "ds/app/engine/engine_roots.h"

#include "ds/app/engine/engine.h"
#include "ds/app/auto_draw.h"
#include "ds/gl/save_camera.h"

namespace ds {

/**
 * \class ds::EngineRoot
 */
ds::ui::Sprite* EngineRoot::make(ui::SpriteEngine& e, const ds::sprite_id_t id, const bool perspective)	{
	return new ds::ui::Sprite(e, id, perspective);
}

EngineRoot::EngineRoot(const RootList::Root& r, const sprite_id_t id)
		: mRootBuilder(r)
		, mSpriteId(id) {
}

EngineRoot::~EngineRoot() {
}

const RootList::Root& EngineRoot::getBuilder() const {
	return mRootBuilder;
}

/**
 * \class ds::OrthRoot
 */
OrthRoot::OrthRoot(Engine& e, const RootList::Root& r, const sprite_id_t id)
		: inherited(r, id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSetViewport(true)
		, mSprite(EngineRoot::make(e, id, false))
		, mSrcRect(0.0f, 0.0f, -1.0f, -1.0f)
		, mDstRect(0.0f, 0.0f, -1.0f, -1.0f)
		, mNearPlane(-1.0f)
		, mFarPlane(1.0f)
{
}

void OrthRoot::setup(const Settings& s) {
	mSrcRect = s.mSrcRect;
	mDstRect = s.mDstRect;

	// Src rect and dst rect is the new way of managing screen size, offset and position.
	if (mSrcRect.x2 > mSrcRect.x1 && mSrcRect.y2 > mSrcRect.y1
			&& mDstRect.x2 > mDstRect.x1 && mDstRect.y2 > mDstRect.y1) {
		mSprite->setSize(mSrcRect.getWidth(), mSrcRect.getHeight());
	} else {
		// This is deprecated, and should never be hit
		const bool			scaleWorldToFit = s.mDebugSettings.getBool("scale_world_to_fit", 0, false);
		const float			window_scale = s.mDebugSettings.getFloat("window_scale", 0, s.mDefaultScale);

		mSprite->setSize(s.mScreenRect.getWidth(), s.mScreenRect.getHeight());
		if (scaleWorldToFit) {
			mSprite->setScale(mEngine.getWidth()/mEngine.getWorldWidth(), mEngine.getHeight()/mEngine.getWorldHeight());
		} else if (window_scale != s.mDefaultScale) {
			mSprite->setScale(window_scale, window_scale);
		}
	}
}

void OrthRoot::postAppSetup() {
	mSprite->postAppSetup();
}

void OrthRoot::slaveTo(EngineRoot*) {
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

void OrthRoot::drawClient(const DrawParams& p, AutoDrawService* auto_draw) {
	if (mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	ci::Matrix44f		m(ci::gl::getModelView());
	// Account for src rect translation
	if (mSrcRect.x2 > mSrcRect.x1 && mSrcRect.y2 > mSrcRect.y1) {
		const float			sx = mDstRect.getWidth() / mSrcRect.getWidth(),
							sy = mDstRect.getHeight() / mSrcRect.getHeight();
		m.translate(ci::Vec3f(-mSrcRect.x1*sx, -mSrcRect.y1*sy, 0.0f));
		m.scale(ci::Vec3f(sx, sy, 1.0f));
	}
	mSprite->drawClient(m, p);

	if (auto_draw) auto_draw->drawClient(m, p);
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
		ci::gl::setViewport(ci::Area((int)screen_rect.getX1(), (int)screen_rect.getY2(), (int)screen_rect.getX2(), (int)screen_rect.getY1()));
	}
	mCamera.setOrtho(screen_rect.getX1(), screen_rect.getX2(), screen_rect.getY2(), screen_rect.getY1(), mNearPlane, mFarPlane);
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
		ci::gl::setViewport(ci::Area((int)screen_rect.getX1(), (int)screen_rect.getY2(), (int)screen_rect.getX2(), (int)screen_rect.getY1()));
	}
	ci::gl::setMatrices(mCamera);
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
}

/**
 * \class ds::PerspRoot
 */
PerspRoot::PerspRoot(Engine& e, const RootList::Root& r, const sprite_id_t id, const PerspCameraParams& p, Picking* picking)
		: inherited(r, id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSprite(EngineRoot::make(e, id, true))
		, mMaster(nullptr)
		, mOldPick(mCamera)
		, mPicking(picking ? *picking : mOldPick) {
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

void PerspRoot::postAppSetup() {
	mSprite->postAppSetup();
}

void PerspRoot::slaveTo(EngineRoot* r) {
	mMaster = dynamic_cast<PerspRoot*>(r);
	if (!mMaster) return;
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

void PerspRoot::drawClient(const DrawParams& p, AutoDrawService* auto_draw) {
	drawFunc([this, &p](){mSprite->drawClient(ci::gl::getModelView(), p);});

	if (auto_draw) auto_draw->drawClient(ci::gl::getModelView(), p);
}

void PerspRoot::drawServer(const DrawParams& p) {
	// Redirect to client draw for now
	drawFunc([this, &p](){mSprite->drawClient(ci::gl::getModelView(), p);});
}

ui::Sprite* PerspRoot::getHit(const ci::Vec3f& point) {
	ui::Sprite*		s = nullptr;
	drawFunc([this, &point, &s](){s = mPicking.pickAt(point.xy(), *(mSprite.get()));});
	return s;
}

PerspCameraParams PerspRoot::getCamera() const {
	if (mMaster) return mMaster->getCamera();

	PerspCameraParams		p;
	p.mPosition = mCamera.getEyePoint();
	p.mTarget = mCamera.getCenterOfInterestPoint();
	p.mFov = mCamera.getFov();
	p.mNearPlane = mCamera.getNearClip();
	p.mFarPlane = mCamera.getFarClip();
	p.mLensShiftH = mCamera.getLensShiftHorizontal();
	p.mLensShiftV = mCamera.getLensShiftVertical();
	return p;
}

const ci::CameraPersp& PerspRoot::getCameraRef() const {
	if (mMaster) return mMaster->getCameraRef();

	return mCamera;
}

void PerspRoot::setCamera(const PerspCameraParams& p) {
	if (mMaster) {
#ifdef _DEBUG
		throw std::runtime_error("PerspRoot::setCamera() illegal: root is a slave");
#endif
		return;
	}
	if (p == getCamera()) return;

	mCamera.setEyePoint(p.mPosition);
	mCamera.setCenterOfInterestPoint(p.mTarget);
	mCamera.setPerspective(p.mFov, ci::app::getWindowAspectRatio(), p.mNearPlane, p.mFarPlane);
	mCamera.setLensShiftHorizontal(p.mLensShiftH);
	mCamera.setLensShiftVertical(p.mLensShiftV);

	//mCamera.setLensShiftHorizontal(1.0f);

	mCameraDirty = true;
}

void PerspRoot::markCameraDirty() {
	mCameraDirty = true;
}

void PerspRoot::setCinderCamera() {
	mCameraDirty = false;

	if (mMaster) {
		if (mMaster->mCameraDirty) mMaster->setCinderCamera();
	} else {
		mCamera.setPerspective(mCamera.getFov(), ci::app::getWindowAspectRatio(), mCamera.getNearClip(), mCamera.getFarClip());
	}
}

void PerspRoot::setGlCamera() {
	if (mMaster) {
		ci::gl::setMatrices(mMaster->mCamera);
	} else {
		ci::gl::setMatrices(mCamera);
	}
	// enable the depth buffer (after all, we are doing 3D)
	//gl::enableDepthRead();
	//gl::enableDepthWrite();
	//gl::translate(-getWorldWidth()/2.0f, -getWorldHeight()/2.0f, 0.0f);
}

void PerspRoot::drawFunc(const std::function<void(void)>& fn) {
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

	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();
	glClear(GL_DEPTH_BUFFER_BIT);

	fn();
}

/**
 * \class ds::PerspRoot::OldPick
 */
PerspRoot::OldPick::OldPick(ci::Camera& c)
		: mCamera(c) {
}

ds::ui::Sprite* PerspRoot::OldPick::pickAt(const ci::Vec2f& pt, ds::ui::Sprite& root) {
	ds::CameraPick			pick(mCamera, ci::Vec3f(pt.x, pt.y, 0.0f), root.getWidth(), root.getHeight());
	return root.getPerspectiveHit(pick);
}

} // namespace ds
