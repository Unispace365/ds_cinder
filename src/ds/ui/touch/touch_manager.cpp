#include "touch_manager.h"
#include "touch_info.h"
#include "ds/app/engine/engine.h"
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
{
  mTouchColor = Color( 1, 1, 0 );
}

void TouchManager::touchesBegin( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
    TouchInfo touchInfo;
    touchInfo.mCurrentGlobalPoint = Vec3f(touchIt->getPos(), 0.0f);
    touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
    mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
    touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Added;

    Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
    touchInfo.mPickedSprite = currentSprite;

    if ( currentSprite ) {
      mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
      currentSprite->processTouchInfo(touchInfo);
    }

    //std::cout << "touch began: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

void TouchManager::touchesMoved( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
    TouchInfo touchInfo;
    touchInfo.mCurrentGlobalPoint = Vec3f(touchIt->getPos(), 0.0f);
    touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
    touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Moved;
    touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

    if (fabs(touchInfo.mDeltaPoint.length()) < math::EPSILON)
      continue;

    if (mFingerDispatcher[touchInfo.mFingerId]) {
      mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
    }

    mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;

    //std::cout << "touch moved: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

void TouchManager::touchesEnded( TouchEvent event )
{
  for (std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt) {
    TouchInfo touchInfo;
    touchInfo.mCurrentGlobalPoint = Vec3f(touchIt->getPos(), 0.0f);
    touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
    touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
    touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
    touchInfo.mPhase = TouchInfo::Removed;
    touchInfo.mPickedSprite = nullptr;

    if (mFingerDispatcher[touchInfo.mFingerId]) {
      mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
      mFingerDispatcher[touchInfo.mFingerId] = nullptr;
    }

    mTouchStartPoint.erase(touchInfo.mFingerId);
    mTouchPreviousPoint.erase(touchInfo.mFingerId);
    mFingerDispatcher.erase(touchInfo.mFingerId);
    //std::cout << "touch ended: " << touchIt->getId() << " @: x: " << touchIt->getPos().x << " | y: " << touchIt->getPos().y << std::endl;
  }
}

void TouchManager::mouseTouchBegin( MouseEvent event, int id )
{
  if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch())
    return;

  TouchInfo touchInfo;
  touchInfo.mCurrentGlobalPoint = Vec3f(event.getPos(), 0.0f);
  touchInfo.mFingerId = id;
  touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
  mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;
  touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
  touchInfo.mPhase = TouchInfo::Added;

  Sprite *currentSprite = getHit(touchInfo.mCurrentGlobalPoint);
  touchInfo.mPickedSprite = currentSprite;

  if ( currentSprite ) {
    mFingerDispatcher[touchInfo.mFingerId] = currentSprite;
    currentSprite->processTouchInfo(touchInfo);
  }

  //std::cout << "mouse began: " << id << " @: x: " << touchInfo.mCurrentGlobalPoint.x << " | y: " << touchInfo.mCurrentGlobalPoint.y << std::endl;
}

void TouchManager::mouseTouchMoved( MouseEvent event, int id )
{
  if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch())
    return;

  TouchInfo touchInfo;
  touchInfo.mCurrentGlobalPoint = Vec3f(event.getPos(), 0.0f);
  touchInfo.mFingerId = id;
  touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
  touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
  touchInfo.mPhase = TouchInfo::Moved;
  touchInfo.mPickedSprite = mFingerDispatcher[touchInfo.mFingerId];

  if (fabs(touchInfo.mDeltaPoint.length()) < math::EPSILON)
    return;

  if (mFingerDispatcher[touchInfo.mFingerId]) {
    mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
  }

  mTouchPreviousPoint[touchInfo.mFingerId] = touchInfo.mCurrentGlobalPoint;

  //std::cout << "mouse moved: " << id << " @: x: " << touchInfo.mCurrentGlobalPoint.x << " | y: " << touchInfo.mCurrentGlobalPoint.y << std::endl;
}

void TouchManager::mouseTouchEnded( MouseEvent event, int id )
{
  if (mEngine.systemMultitouchEnabled() && ci::System::hasMultiTouch())
    return;

  TouchInfo touchInfo;
  touchInfo.mCurrentGlobalPoint = Vec3f(event.getPos(), 0.0f);
  touchInfo.mFingerId = id;
  touchInfo.mStartPoint = mTouchStartPoint[touchInfo.mFingerId];
  touchInfo.mDeltaPoint = touchInfo.mCurrentGlobalPoint - mTouchPreviousPoint[touchInfo.mFingerId];
  touchInfo.mPhase = TouchInfo::Removed;
  touchInfo.mPickedSprite = nullptr;

  if (mFingerDispatcher[touchInfo.mFingerId]) {
    mFingerDispatcher[touchInfo.mFingerId]->processTouchInfo( touchInfo );
    mFingerDispatcher[touchInfo.mFingerId] = nullptr;
  }

  mTouchStartPoint.erase(touchInfo.mFingerId);
  mTouchPreviousPoint.erase(touchInfo.mFingerId);
  mFingerDispatcher.erase(touchInfo.mFingerId);
  //std::cout << "mouse ended: " << id << " @: x: " << touchInfo.mCurrentGlobalPoint.x << " | y: " << touchInfo.mCurrentGlobalPoint.y << std::endl;
}

void TouchManager::drawTouches() const
{
  if (mTouchPreviousPoint.empty())
    return;

  applyBlendingMode(NORMAL);
  gl::color( mTouchColor );

  for ( auto it = mTouchPreviousPoint.begin(), it2 = mTouchPreviousPoint.end(); it != it2; ++it ) {
  	gl::drawStrokedCircle( it->second.xy(), 20.0f );
  }
}

void TouchManager::setTouchColor( const ci::Color &color )
{
  mTouchColor = color;
}

void TouchManager::clearFingers( const std::vector<int> &fingers )
{
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

Sprite* TouchManager::getHit(const ci::Vec3f &point)
{
	for (int k=mEngine.getRootCount()-1; k>=0; --k) {
		ui::Sprite&						root(mEngine.getRootSprite(k));
		Sprite*								s = nullptr;
		if (root.getPerspective()) {
			ds::CameraPick			pick(mEngine.getPerspectiveCamera(), point, root.getWidth(), root.getHeight());
			s = root.getPerspectiveHit(pick);
		} else {
			s = root.getHit(point);
		}
		if (s) return s;
	}
  return nullptr;
}


} // namespace ui
} // namespace ds
