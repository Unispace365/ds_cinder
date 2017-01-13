#include "ds/app/engine/engine_roots.h"

#include "ds/app/engine/engine.h"
#include "ds/app/auto_draw.h"

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
		, mSetViewport(false)
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
	if(mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	ci::mat4 m = ci::gl::getModelMatrix();
	mSprite->drawClient(m, p);

	if (auto_draw) auto_draw->drawClient(m, p);

	if(mSetViewport){
		ci::gl::context()->popViewport();
	}
}

void OrthRoot::drawServer(const DrawParams& p) {
	if(mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	ci::mat4 m = ci::gl::getModelMatrix();
	mSprite->drawServer(m, p);

	if(mSetViewport){
		ci::gl::context()->popViewport();
	}
}

ui::Sprite* OrthRoot::getHit(const ci::vec3& point) {
	return mSprite->getHit(point);
}

void OrthRoot::setCinderCamera() {
	mCameraDirty = false;

	if(getBuilder().mDrawScaled) {
		mCamera.setOrtho(mSrcRect.x1, mSrcRect.x2, mSrcRect.y2, mSrcRect.y1, mNearPlane, mFarPlane);
	}
	else {
		mCamera.setOrtho(0.0f, mDstRect.getWidth(), mDstRect.getHeight(), 0.0f, mNearPlane, mFarPlane);
	}
}

void OrthRoot::setViewport(const bool b) {
	mSetViewport = b;
}

void OrthRoot::markCameraDirty() {
	mCameraDirty = true;
}

void OrthRoot::setGlCamera() {
	if (mSetViewport) {
		ci::gl::context()->pushViewport(std::make_pair<ci::vec2, ci::vec2>(ci::vec2(0, (int)mDstRect.getHeight()), ci::vec2((int)mDstRect.getWidth(), 0)));
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
	mCamera.lookAt(p.mTarget);
	mCamera.setFov(p.mFov);
	mCamera.setNearClip(p.mNearPlane);
	mCamera.setFarClip(p.mFarPlane);
	mCameraTarget = p.mTarget;
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

ui::Sprite* PerspRoot::getHit(const ci::vec3& point) {
	ui::Sprite*		s = nullptr;
	drawFunc([this, &point, &s](){s = mPicking.pickAt(glm::vec2(point), *(mSprite.get())); });
	return s;
}

PerspCameraParams PerspRoot::getCamera() const {
	if (mMaster) return mMaster->getCamera();

	PerspCameraParams		p;
	p.mPosition = mCamera.getEyePoint();
	p.mTarget = mCameraTarget;
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

void PerspRoot::setCameraRef(const ci::CameraPersp& cam){
	mCamera = cam;
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
	mCamera.lookAt(p.mTarget);
	mCameraTarget = p.mTarget;
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
	ci::gl::pushMatrices();

	if (mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	glClear(GL_DEPTH_BUFFER_BIT);

	fn();
	ci::gl::popMatrices();
}

/**
 * \class ds::PerspRoot::OldPick
 */
PerspRoot::OldPick::OldPick(ci::Camera& c)
		: mCamera(c) {
}

ds::ui::Sprite* PerspRoot::OldPick::pickAt(const ci::vec2& pt, ds::ui::Sprite& root) {
	ds::CameraPick			pick(mCamera, ci::vec3(pt.x, pt.y, 0.0f), root.getWidth(), root.getHeight());
	return root.getPerspectiveHit(pick);
}

} // namespace ds
