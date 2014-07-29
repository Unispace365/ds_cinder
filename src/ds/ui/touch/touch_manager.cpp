#include "touch_manager.h"
#include "touch_info.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_roots.h"
#include "ds/math/math_defs.h"
#include "ds/ui/sprite/util/blend.h"
#include <cinder/System.h>

namespace {
  const int MOUSE_RESERVED_IDS = 2;
}

namespace ds {
namespace ui {

TouchManager::TouchManager( Engine &engine )
  : mEngine(engine)
  , mIgnoreFirstTouchId(-1)
  , mOverrideTranslation(false)
  , mTouchDimensions(0.0f, 0.0f)
  , mTouchOffset(0.0f, 0.0f)
  , mTouchFilterRect(0.0f, 0.0f, 0.0f, 0.0f)
{
  mTouchColor = Color( 1, 1, 0 );
}

void TouchManager::touchesBegin( TouchEvent event ){
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		// This system uses a mouse click for the first touch, which allows for use of the mouse and touches simultaneously
		// It's possible we'll run into a scenario where we need to reverse this, which we can just add a bool flag to the settings to use all touches and ignore all mouses.
		if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch() && mIgnoreFirstTouchId < 0){
			mIgnoreFirstTouchId = touchIt->getId();
			return;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		if (shouldDiscardTouch(touchPos))
			return;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Added;
		touchInfo.mPassedTouch = false;

		if(mEngine.getUseTouchTrails()){
			mTouchPointHistory[touchInfo.mFingerId] = std::vector<ci::Vec3f>();
			mTouchPointHistory[touchInfo.mFingerId].push_back(touchInfo.mCurrentGlobalPoint);
		}

		Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
		touchInfo.mPickedSprite = currentSprite;

		if ( currentSprite ) {
			mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
			currentSprite->processTouchInfo(touchInfo);
		}

	}
}

void TouchManager::touchesMoved( TouchEvent event ){
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId){
			continue;
		}

		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		if (shouldDiscardTouch(touchPos))
			return;

		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchPos, 0.0f);
		touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Moved;
		touchInfo.mPassedTouch = false;
		touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

		if(mEngine.getUseTouchTrails()){
			mTouchPointHistory[touchInfo.mFingerId].push_back(touchInfo.mCurrentGlobalPoint);
			if((int)mTouchPointHistory[touchInfo.mFingerId].size() > mEngine.getTouchTrailLength() - 1){
				mTouchPointHistory[touchInfo.mFingerId].erase(mTouchPointHistory[touchInfo.mFingerId].begin());
			}
		}

		if (mFingerDispatcher[touchInfo.mFingerId]) {
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
		}

		mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	}
}

void TouchManager::touchesEnded( TouchEvent event ){
	for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

		if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch() && touchIt->getId() == mIgnoreFirstTouchId){
			mIgnoreFirstTouchId = -1;
			continue;
		}


		ci::Vec2f touchPos = touchIt->getPos();
		if(mOverrideTranslation){
			overrideTouchTranslation(touchPos);
		}

		if (shouldDiscardTouch(touchPos))
			return;


		TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = Vec3f(touchIt->getPos(), 0.0f);
		touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
		touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
		touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
		touchInfo.mPhase = TouchInfo::Removed;
		touchInfo.mPassedTouch = false;
		touchInfo.mPickedSprite = nullptr;

		if (mFingerDispatcher[touchInfo.mFingerId]) {
			mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
			mFingerDispatcher[touchInfo.mFingerId] = nullptr;
		}

		mTouchStartPoint.erase(touchInfo.mFingerId);
		mTouchPreviousPoint.erase(touchInfo.mFingerId);
		mFingerDispatcher.erase(touchInfo.mFingerId);

		if(mEngine.getUseTouchTrails()){
			mTouchPointHistory.erase(touchInfo.mFingerId);
		}
	}
}

void TouchManager::mouseTouchBegin( MouseEvent event, int id ){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Added;
	touchInfo.mPassedTouch = false;

	if(mEngine.getUseTouchTrails()){
		mTouchPointHistory[touchInfo.mFingerId] = std::vector<ci::Vec3f>();
		mTouchPointHistory[touchInfo.mFingerId].push_back(touchInfo.mCurrentGlobalPoint);
	}

	Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
	touchInfo.mPickedSprite = currentSprite;

	if ( currentSprite ) {
		mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
		currentSprite->processTouchInfo(touchInfo);
	}
}

void TouchManager::mouseTouchMoved( MouseEvent event, int id ){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Moved;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];


	if(mEngine.getUseTouchTrails()){
		mTouchPointHistory[touchInfo.mFingerId].push_back(touchInfo.mCurrentGlobalPoint);
		if((int)mTouchPointHistory[touchInfo.mFingerId].size() > mEngine.getTouchTrailLength() - 1 ){
			mTouchPointHistory[touchInfo.mFingerId].erase(mTouchPointHistory[touchInfo.mFingerId].begin());
		}
	}

	if (mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
	}

	mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
}

void TouchManager::mouseTouchEnded( MouseEvent event, int id ){

	TouchInfo touchInfo;
	touchInfo.mCurrentGlobalPoint = Vec3f(translateMousePoint(event.getPos()), 0.0f);
	touchInfo.mFingerId = id;
	touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
	touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
	touchInfo.mPhase = TouchInfo::Removed;
	touchInfo.mPassedTouch = false;
	touchInfo.mPickedSprite = nullptr;

	if (mFingerDispatcher[touchInfo.mFingerId]) {
		mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
		mFingerDispatcher[touchInfo.mFingerId] = nullptr;
	}

	mTouchStartPoint.erase(touchInfo.mFingerId);
	mTouchPreviousPoint.erase(touchInfo.mFingerId);
	mFingerDispatcher.erase(touchInfo.mFingerId);

	if(mEngine.getUseTouchTrails()){
		mTouchPointHistory.erase(touchInfo.mFingerId);
	}
}

void TouchManager::drawTouches() const {
	if(mEngine.getUseTouchTrails()){
		applyBlendingMode(NORMAL);
		ci::gl::color( mTouchColor );

		const float incrementy = mEngine.getTouchTrailIncrement();
		ci::Vec2f			mouse_offset(mEngine.getMouseOffset());
		for ( auto it = mTouchPointHistory.begin(), it2 = mTouchPointHistory.end(); it != it2; ++it ) {
			float sizey = incrementy;
			int secondSize = it->second.size();
			ci::Vec2f prevPos = ci::Vec2f::zero();
			for (int i = 0; i < secondSize; i++){
				ci::Vec2f		pos(it->second[i].xy());
				pos -= mouse_offset;
				ci::gl::drawSolidCircle(pos, sizey);

				if(i < secondSize - 1 && i > 0){ 
					// Find the angle between this point and the previous point
					// PI / 2 is a 90 degree rotation, or perpendicular
					float angle = atan2f(pos.y - prevPos.y, pos.x - prevPos.x) + ds::math::PI / 2.0f;
					float smallSize = (sizey - incrementy);
					float bigSize = sizey;
					ci::Vec2f p1 = ci::Vec2f(pos.x + bigSize * cos(angle), pos.y + bigSize * sin(angle));
					ci::Vec2f p2 = ci::Vec2f(pos.x - bigSize * cos(angle), pos.y - bigSize * sin(angle));
					ci::Vec2f p3 = ci::Vec2f(prevPos.x + smallSize * cos(angle), prevPos.y + smallSize * sin(angle));
					ci::Vec2f p4 = ci::Vec2f(prevPos.x - smallSize * cos(angle), prevPos.y - smallSize * sin(angle));
					glBegin(GL_QUADS);
					ci::gl::vertex(p1);
					ci::gl::vertex(p3);
					ci::gl::vertex(p4);
					ci::gl::vertex(p2);
					glEnd();
				}

				sizey += incrementy;

				prevPos = pos;
			}
		}

	} else {
		if (mTouchPreviousPoint.empty())
			return;

		applyBlendingMode(NORMAL);
		ci::gl::color( mTouchColor );

		ci::Vec2f			mouse_offset(mEngine.getMouseOffset());
		for ( auto it = mTouchPreviousPoint.begin(), it2 = mTouchPreviousPoint.end(); it != it2; ++it ) {
			ci::Vec2f		pos(it->second.xy());
			pos.x -= mouse_offset.x;
			pos.y -= mouse_offset.y;
			ci::gl::drawStrokedCircle(pos, 20.0f);
		}
	}
}

void TouchManager::setTouchColor( const ci::Color &color ){
	mTouchColor = color;
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
	
	float yScaleFactor = getWindowHeight() / mEngine.getScreenRect().getHeight();
	float xScaleFactor = getWindowWidth() / mEngine.getScreenRect().getWidth();
	ci::Vec2f eventPos = ci::Vec2f((float)inputPoint.x, (float)inputPoint.y);
	eventPos.x /= xScaleFactor;
	eventPos.y /= yScaleFactor;

	const ci::Vec2i		mouseOffset(mEngine.getMouseOffset());
	return eventPos;
}

bool TouchManager::shouldDiscardTouch( const ci::Vec2f& p ) {
	if (mTouchFilterRect.getWidth() != 0.0f ) {
		return !mTouchFilterRect.contains(p);
	}
	return false;
}

void TouchManager::overrideTouchTranslation( ci::Vec2f& inOutPoint){
	inOutPoint.set((inOutPoint.x / getWindowWidth()) * mTouchDimensions.x + mTouchOffset.x, 
		(inOutPoint.y / getWindowHeight()) * mTouchDimensions.y + mTouchOffset.y);
}


} // namespace ui
} // namespace ds
