#include "touch_manager.h"

#include <cinder/System.h>

#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_roots.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/ui/touch/touch_info.h"
#include "ds/ui/touch/touch_event.h"
#include "ds/ui/touch/rotation_translator.h"

namespace {
  const int MOUSE_RESERVED_IDS = 2;
}

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

void TouchManager::touchesBegin(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		// This system uses a mouse click for the first touch, which allows for use of the mouse and touches simultaneously
		// It's possible we'll run into a scenario where we need to reverse this, which we can just add a bool flag to the settings to use all touches and ignore all mouses.
		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && TouchMode::hasMouse(mTouchMode)) {
			mIgnoreFirstTouchId = touchIt->getId();
			return;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
  
		if(shouldDiscardTouch(touchPos)){
			mDiscardTouchMap[fingerId] = true;
			continue;
		}

		mDiscardTouchMap[fingerId] = false;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = ci::Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = fingerId;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];

		// Catch a case where two "touch added" calls get processed for the same fingerID
		// WITHOUT a released in the middle. This would case the previous sprite to be left with an erroneous finger
		// So we fake remove it before adding the new one
		if(mFingerDispatcher[touchInfo.mFingerId]) {
			DS_LOG_WARNING("Double touch added on the same finger Id: " << touchInfo.mFingerId << ", removing previous sprite tracking.");
			touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];
			touchInfo.mPhase = TouchInfo::Removed; // fake removed
			touchInfo.mPassedTouch = true; // passed touch flag indicates that this info shouldn't be used to trigger buttons, etc. implementation up to each sprite
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo(touchInfo);
			mFingerDispatcher[touchInfo.mFingerId] = nullptr;

		}

		touchInfo.mPhase = TouchInfo::Added;
		touchInfo.mPassedTouch = false;

		if(mCapture) mCapture->touchBegin(touchInfo);

		Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
		touchInfo.mPickedSprite = currentSprite;
		mRotationTranslator.down(touchInfo);

		if ( currentSprite ) {
			mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
			currentSprite->processTouchInfo(touchInfo);
		}
	}
}

void TouchManager::touchesMoved(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId) {
			continue;
		}

		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		//if (shouldDiscardTouch(touchPos))
		//	return;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = ci::Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = fingerId;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Moved;
		touchInfo.mPassedTouch = false;
		touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

		if(mCapture){
			mCapture->touchMoved(touchInfo);
		}

		mRotationTranslator.move(touchInfo, mTouchPreviousPoint[touchInfo.mFingerId]);

		if (mFingerDispatcher[touchInfo.mFingerId]) {
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
		}

		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	}
}

void TouchManager::touchesEnded(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (TouchMode::hasSystem(mTouchMode) && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId){
			mIgnoreFirstTouchId = -1;
			continue;
		}


		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		//if (shouldDiscardTouch(touchPos))
		//	return;


		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = ci::Vec3f(touchPos, 0.0f);
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

void TouchManager::mouseTouchBegin(const ci::app::MouseEvent &event, int id ){
	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = ci::Vec3f(translateMousePoint(event.getPos()), 0.0f);
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

void TouchManager::mouseTouchMoved(const ci::app::MouseEvent &event, int id){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = ci::Vec3f(translateMousePoint(event.getPos()), 0.0f);
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

void TouchManager::mouseTouchEnded(const ci::app::MouseEvent &event, int id){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = ci::Vec3f(translateMousePoint(event.getPos()), 0.0f);
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
	inOutPoint.set((inOutPoint.x / ci::app::getWindowWidth()) * mTouchDimensions.x + mTouchOffset.x, 
		(inOutPoint.y / ci::app::getWindowHeight()) * mTouchDimensions.y + mTouchOffset.y);
}


} // namespace ui
} // namespace ds
