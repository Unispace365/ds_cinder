#include "touch_manager.h"
#include "touch_info.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_roots.h"
#include "ds/math/math_defs.h"
#include "ds/ui/sprite/util/blend.h"
#include <cinder/System.h>
#include "rotation_translator.h"

namespace {
  const int MOUSE_RESERVED_IDS = 2;
}

using namespace ci;
using namespace ci::app;

namespace ds {
namespace ui {

TouchManager::TouchManager(Engine &engine, const TouchMode::Enum &mode)
		: mEngine(engine)
		, mTouchDimensions(0.0f, 0.0f)
		, mTouchOffset(0.0f, 0.0f)
		, mOverrideTranslation(false)
		, mTouchFilterRect(0.0f, 0.0f, 0.0f, 0.0f)
		, mTouchMode(mode)
		, mIgnoreFirstTouchId(-1)
		, mCapture(nullptr)
		, mRotationTranslatorPtr(new RotationTranslator())
		, mRotationTranslator(*(mRotationTranslatorPtr.get()))
{
}

void TouchManager::setTouchMode(const TouchMode::Enum &m) {
	mTouchMode = m;
}

void TouchManager::touchesBegin(const TouchEvent &event) {
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		// This system uses a mouse click for the first touch, which allows for use of the mouse and touches simultaneously
		// It's possible we'll run into a scenario where we need to reverse this, which we can just add a bool flag to the settings to use all touches and ignore all mouses.
		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && TouchMode::hasMouse(mTouchMode)) {
			mIgnoreFirstTouchId = touchIt->getId();
			return;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
  
		if(shouldDiscardTouch(touchPos)){
			mDiscardTouchMap[fingerId] = true;
			continue;
		}

		mDiscardTouchMap[fingerId] = false;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = fingerId;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Added;
		touchInfo.mPassedTouch = false;

		if (mCapture) mCapture->touchBegin(touchInfo);

		Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
		touchInfo.mPickedSprite = currentSprite;
		mRotationTranslator.down(touchInfo);

		if ( currentSprite ) {
			mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
			currentSprite->processTouchInfo(touchInfo);
		}
	}
}

void TouchManager::touchesMoved(const TouchEvent &event) {
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId) {
			continue;
		}

		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		//if (shouldDiscardTouch(touchPos))
		//	return;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = fingerId;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Moved;
		touchInfo.mPassedTouch = false;
		touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];
		mRotationTranslator.move(touchInfo, mTouchPreviousPoint[touchInfo.mFingerId]);

		if(mCapture){
			mCapture->touchMoved(touchInfo);
		}

		if (mFingerDispatcher[touchInfo.mFingerId]) {
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
		}

		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	}
}

void TouchManager::touchesEnded(const TouchEvent &event) {
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId){
			mIgnoreFirstTouchId = -1;
			continue;
		}


		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		//if (shouldDiscardTouch(touchPos))
		//	return;


		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = fingerId;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Removed;
		touchInfo.mPassedTouch = false;
		touchInfo.mPickedSprite = nullptr;
		mRotationTranslator.up(touchInfo);
	
		if (mFingerDispatcher[touchInfo.mFingerId]) {
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
			mFingerDispatcher[touchInfo.mFingerId] = nullptr;
		}

		mTouchStartPoint.erase(touchInfo.mFingerId);
		mTouchPreviousPoint.erase(touchInfo.mFingerId);
		mFingerDispatcher.erase(touchInfo.mFingerId);

		if (mCapture) mCapture->touchEnd(touchInfo);
	}
}

void TouchManager::mouseTouchBegin(const MouseEvent &event, int id ){
	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Added;
	touchInfo.mPassedTouch = false;

	if (mCapture) mCapture->touchBegin(touchInfo);

	Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
	touchInfo.mPickedSprite = currentSprite;
	mRotationTranslator.down(touchInfo);

	if ( currentSprite ) {
		mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
		currentSprite->processTouchInfo(touchInfo);
	}
}

void TouchManager::mouseTouchMoved(const MouseEvent &event, int id ){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Moved;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

	if(mCapture) mCapture->touchMoved(touchInfo);

	mRotationTranslator.move(touchInfo, mTouchPreviousPoint[touchInfo.mFingerId]);

	if (mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
	}

	mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
}

void TouchManager::mouseTouchEnded(const MouseEvent &event, int id ){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Removed;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = nullptr;
	mRotationTranslator.up(touchInfo);

	if (mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
		mFingerDispatcher[touchInfo.mFingerId] = nullptr;
	}

	mTouchStartPoint.erase(touchInfo.mFingerId);
	mTouchPreviousPoint.erase(touchInfo.mFingerId);
	mFingerDispatcher.erase(touchInfo.mFingerId);

	if (mCapture) mCapture->touchEnd(touchInfo);
}

void TouchManager::drawTouches() const {
	if (mTouchPreviousPoint.empty())
		return;

	applyBlendingMode(NORMAL);

	for ( auto it = mTouchPreviousPoint.begin(), it2 = mTouchPreviousPoint.end(); it != it2; ++it ) {
		ci::Vec2f		pos(it->second.xy());
		ci::gl::drawStrokedCircle(pos, 20.0f);
	}
}

void TouchManager::clearFingers( const std::vector<int> &fingers ){
	for ( auto i = fingers.begin(), e = fingers.end(); i != e; ++i )
	{
		auto dispatcher = mFingerDispatcher.find(*i);
		if ( dispatcher != mFingerDispatcher.end() )
			mFingerDispatcher.erase(dispatcher);

		auto startPoint = mTouchStartPoint.find(*i);
		if ( startPoint != mTouchStartPoint.end() )
			mTouchStartPoint.erase(startPoint);

		auto prevPoint = mTouchPreviousPoint.find(*i);
		if ( prevPoint != mTouchPreviousPoint.end() )
			mTouchPreviousPoint.erase(prevPoint);
	}
}

void TouchManager::setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ){
	if (!theSprite){
		return;
	}
	
	mFingerDispatcher[fingerId] = theSprite;
}

Sprite* TouchManager::getSpriteForFinger( const int fingerId ){
	return mFingerDispatcher[fingerId];
}

Sprite* TouchManager::getHit(const ci::Vec3f &point) {
	return mEngine.getHit(point);
}

ci::Vec2f TouchManager::translateMousePoint( const ci::Vec2i inputPoint ){
	// The translation has been moved to the engine
	return ci::Vec2f(static_cast<float>(inputPoint.x), static_cast<float>(inputPoint.y));
}

bool TouchManager::shouldDiscardTouch( const ci::Vec2f& p ) {
	if (mTouchFilterRect.getWidth() != 0.0f ) {
		return !mTouchFilterRect.contains(p);
	}
	return false;
}

void TouchManager::setCapture(Capture *c) {
	mCapture = c;
}

void TouchManager::overrideTouchTranslation( ci::Vec2f& inOutPoint){
	inOutPoint.set((inOutPoint.x / getWindowWidth()) * mTouchDimensions.x + mTouchOffset.x, 
		(inOutPoint.y / getWindowHeight()) * mTouchDimensions.y + mTouchOffset.y);
}


} // namespace ui
} // namespace ds
