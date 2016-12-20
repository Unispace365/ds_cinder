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
		, mTouchFilterFunc(nullptr)
		, mTouchMode(mode)
		, mCapture(nullptr)
		, mRotationTranslatorPtr(new RotationTranslator())
		, mRotationTranslator(*(mRotationTranslatorPtr.get()))
		, mSmoothEnabled(true)
		, mFramesToSmooth(8)
{
}

void TouchManager::setTouchMode(const TouchMode::Enum &m) {
	mTouchMode = m;
}

void TouchManager::touchesBegin(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		ci::vec2 touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(shouldDiscardTouch(touchPos)){
			mDiscardTouchMap[fingerId] = true;
			continue;
		}

		inputBegin(fingerId, touchPos);
	}
}


void TouchManager::mouseTouchBegin(const ci::app::MouseEvent &event, int id){
	ci::vec2 globalPos = translateMousePoint(event.getPos());

	if(shouldDiscardTouch(globalPos)){
		return;
	}
	inputBegin(id, globalPos);
}

void TouchManager::inputBegin(const int fingerId, const ci::vec2& touchPos){
	mDiscardTouchMap[fingerId] = false;

	ci::vec3 globalPoint = ci::vec3(touchPos, 0.0f);

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = globalPoint;
	touchInfo.mFingerId = fingerId;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	mTouchPreviousPoint[fingerId] = globalPoint;
	touchInfo.mDeltaPoint = ci::vec3();

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

		if(mEngine.getTouchInfoPipeCallback()){
			mEngine.getTouchInfoPipeCallback()(touchInfo);
		}

	}

	touchInfo.mPhase = TouchInfo::Added;
	touchInfo.mPassedTouch = false;

	if(mCapture) mCapture->touchBegin(touchInfo);

	Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
	touchInfo.mPickedSprite = currentSprite;
	mRotationTranslator.down(touchInfo);

	if(mSmoothEnabled){
		mTouchSmoothPoints[fingerId].clear();
		mTouchSmoothPoints[fingerId].push_back(touchInfo.mCurrentGlobalPoint);
	}

	if(currentSprite) {
		mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
		currentSprite->processTouchInfo(touchInfo);
	}

	if(mEngine.getTouchInfoPipeCallback()){
		mEngine.getTouchInfoPipeCallback()(touchInfo);
	}
}



void TouchManager::touchesMoved(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::vec2 touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		inputMoved(fingerId, touchPos);
	}
}

void TouchManager::mouseTouchMoved(const ci::app::MouseEvent &event, int id){
	ci::vec2 globalPos = translateMousePoint(event.getPos());
	inputMoved(id, globalPos);
}

void TouchManager::inputMoved(const int fingerId, const ci::vec2& touchPos){

	ci::vec3 globalPoint = ci::vec3(touchPos, 0.0f);

	if(mSmoothEnabled){
		mTouchSmoothPoints[fingerId].push_back(globalPoint);
		if(mTouchSmoothPoints[fingerId].size() > mFramesToSmooth){
			mTouchSmoothPoints[fingerId].erase(mTouchSmoothPoints[fingerId].begin());
		}
		std::vector<ci::vec3> deltas;
		for(int i = 1; i < mTouchSmoothPoints[fingerId].size(); i++){
			deltas.push_back(mTouchSmoothPoints[fingerId][i] - mTouchSmoothPoints[fingerId][i - 1]);
		}
		float xcomp(0.0f);
		float ycomp(0.0f);
		for(int i = 0; i < deltas.size(); i++){
			xcomp += deltas[i].x;
			ycomp += deltas[i].y;
		}
		xcomp /= deltas.size();
		ycomp /= deltas.size();
		globalPoint = mTouchPreviousPoint[fingerId] + ci::vec3(xcomp, ycomp, 0.0f);
	}

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = globalPoint;
	touchInfo.mFingerId = fingerId;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = globalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Moved;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

	if(mCapture){
		mCapture->touchMoved(touchInfo);
	}

	mRotationTranslator.move(touchInfo, mTouchPreviousPoint[touchInfo.mFingerId]);


	if(mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo(touchInfo);
	}

	mTouchPreviousPoint[touchInfo.mFingerId] = globalPoint;

	if(mEngine.getTouchInfoPipeCallback()){
		mEngine.getTouchInfoPipeCallback()(touchInfo);
	}
}



void TouchManager::touchesEnded(const ds::ui::TouchEvent &event) {
	for (auto touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
		int fingerId = touchIt->getId() + MOUSE_RESERVED_IDS;

		if(mDiscardTouchMap[fingerId]){
			continue;
		}

		ci::vec2 touchPos = touchIt->getPos();
		if(mOverrideTranslation && !event.getInWorldSpace()){
			overrideTouchTranslation(touchPos);
		}

		inputEnded(fingerId, touchPos);
	}
}

void TouchManager::mouseTouchEnded(const ci::app::MouseEvent &event, int id){
	ci::vec2 globalPos = translateMousePoint(event.getPos());
	inputEnded(id, globalPos);
}

void TouchManager::inputEnded(const int fingerId, const ci::vec2& touchPos){
	ci::vec3 globalPoint = ci::vec3(touchPos, 0.0f);

	if(mSmoothEnabled){
		//ignore the smoothing for the end frame and just use the previous point
		globalPoint = mTouchPreviousPoint[fingerId];
	}

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = globalPoint;
	touchInfo.mFingerId = fingerId;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = globalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Removed;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = nullptr;

	mRotationTranslator.up(touchInfo);

	if(mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo(touchInfo);
		mFingerDispatcher[touchInfo.mFingerId] = nullptr;
	}

	mTouchStartPoint.erase(touchInfo.mFingerId);
	mTouchPreviousPoint.erase(touchInfo.mFingerId);
	mFingerDispatcher.erase(touchInfo.mFingerId);

	if(mCapture) mCapture->touchEnd(touchInfo);

	if(mEngine.getTouchInfoPipeCallback()){
		mEngine.getTouchInfoPipeCallback()(touchInfo);
	}
}

void TouchManager::clearFingers( const std::vector<int> &fingers ){
	for ( auto i = fingers.begin(), e = fingers.end(); i != e; ++i )
	{
		auto dispatcher = mFingerDispatcher.find(*i);
		if ( dispatcher != mFingerDispatcher.end() )
			mFingerDispatcher.erase(dispatcher);

		// Testing disabling this part of the clearing.
		// If you disable a sprite during a touch phase, keeping these points around allows 
		// the touch capture drawing to still work correctly, but the sprite won't recieve anything
// 		auto startPoint = mTouchStartPoint.find(*i);
// 		if ( startPoint != mTouchStartPoint.end() )
// 			mTouchStartPoint.erase(startPoint);
// 
// 		auto prevPoint = mTouchPreviousPoint.find(*i);
// 		if ( prevPoint != mTouchPreviousPoint.end() )
// 			mTouchPreviousPoint.erase(prevPoint);
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

Sprite* TouchManager::getHit(const ci::vec3 &point) {
	return mEngine.getHit(point);
}

ci::vec2 TouchManager::translateMousePoint( const ci::ivec2 inputPoint ){
	// The translation has been moved to the engine
	return ci::vec2(static_cast<float>(inputPoint.x), static_cast<float>(inputPoint.y));
}

bool TouchManager::shouldDiscardTouch(const ci::vec2& p) {
	bool output = false;

	if(mTouchFilterRect.getWidth() != 0.0f ) {
		output |= !mTouchFilterRect.contains(p);
	}

	if(mTouchFilterFunc){
		output |= mTouchFilterFunc(p);
	}

	if(!output && mEngine.getMinTouchDistance() > 0.0f){
		for(auto it = mTouchPreviousPoint.begin(); it != mTouchPreviousPoint.end(); ++it){
			if(glm::distance(ci::vec2(it->second), p) < mEngine.getMinTouchDistance()){// 	it->second.xy().distance(p) < mEngine.getMinTouchDistance()){
				output = true;
				break;
			}
		}
	}

	return output;
}

void TouchManager::setCapture(Capture *c) {
	mCapture = c;
}


void TouchManager::setTouchSmoothing(const bool doSmoothing){
	mSmoothEnabled = doSmoothing;
}

void TouchManager::setTouchSmoothFrames(const int smoothFrames){
	mFramesToSmooth = smoothFrames;
}

void TouchManager::overrideTouchTranslation(ci::vec2& inOutPoint){
	inOutPoint.x = (inOutPoint.x / ci::app::getWindowWidth()) * mTouchDimensions.x + mTouchOffset.x;
	inOutPoint.y = (inOutPoint.y / ci::app::getWindowHeight()) * mTouchDimensions.y + mTouchOffset.y;
}


} // namespace ui
} // namespace ds
