#include "touch_manager.h"
#include "touch_info.h"
#include "ds/app/engine.h"
#include "ds/math/math_defs.h"


namespace ds {
namespace ui {

TouchManager::TouchManager( Engine &engine )
  : mEngine(engine)
{

}

void TouchManager::touchesBegin( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
    TouchInfo touchInfo;
    touchInfo.mCurrentPoint = Vec2f(touchIt->getX(), touchIt->getY());
    touchInfo.mFingerId = touchIt->getId();
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentPoint;
    mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentPoint;
    touchInfo.mDeltaPoint = touchInfo.mCurrentPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Added;

    Sprite *currentSprite = mEngine.getRootSprite().getHit(touchInfo.mCurrentPoint);
    touchInfo.mPickedSprite = currentSprite;

    if ( currentSprite ) {
      mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
      //currentSprite->processTouchInfo(touchInfo);
    }

    std::cout << "touch began: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

void TouchManager::touchesMoved( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {

    TouchInfo touchInfo;
    touchInfo.mCurrentPoint = Vec2f(touchIt->getX(), touchIt->getY());
    touchInfo.mFingerId = touchIt->getId();
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
    touchInfo.mDeltaPoint = touchInfo.mCurrentPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Removed;
    touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

    if (fabs(touchInfo.mDeltaPoint.length()) < math::EPSILON)
      continue;

    if (mFingerDispatcher[touchInfo.mFingerId]) {
      //mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
    }

    mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentPoint;

    std::cout << "touch moved: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

void TouchManager::touchesEnded( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
    TouchInfo touchInfo;
    touchInfo.mCurrentPoint = Vec2f(touchIt->getX(), touchIt->getY());
    touchInfo.mFingerId = touchIt->getId();
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
    touchInfo.mDeltaPoint = touchInfo.mCurrentPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Removed;
    touchInfo.mPickedSprite = nullptr;

    if (mFingerDispatcher[touchInfo.mFingerId]) {
      //mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
      mFingerDispatcher[touchInfo.mFingerId] = nullptr;
    }

    mTouchStartPoint.erase(touchInfo.mFingerId);
    mTouchPreviousPoint.erase(touchInfo.mFingerId);
    mFingerDispatcher.erase(touchInfo.mFingerId);
    std::cout << "touch ended: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

} // namespace ui
} // namespace ds
