#include "touch_process.h"
#include "ds/math/math_defs.h"
#include "ds/ui/sprite/sprite.h"
#include "multi_touch_constraints.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

TouchProcess::TouchProcess( SpriteEngine &engine, Sprite &sprite )
  : mSpriteEngine(engine)
  , mSprite(sprite)
  , mTappable(false)
  , mOneTap(false)
{
  mFingers.clear();
}

TouchProcess::~TouchProcess()
{

}

bool TouchProcess::processTouchInfo( const TouchInfo &touchInfo )
{
  if (!mSprite.visible() || !mSprite.isEnabled())
    return false;

  processTap(touchInfo);

  if (TouchInfo::Added == touchInfo.mPhase) {
    mFingers[touchInfo.mFingerId] = touchInfo;

    if (mFingers.size() == 1) {
      mSwipeQueue.clear();
      mSwipeFingerId = touchInfo.mFingerId;
      addToSwipeQueue(touchInfo.mCurrentPoint, 0);
    }

    initializeTouchPoints();
    sendTouchInfo(touchInfo);
    updateDragDestination(touchInfo);

    if (!mSprite.multiTouchEnabled())
      return true;
  } else if (TouchInfo::Moved == touchInfo.mPhase) {
    if (mFingers.empty())
      return false;

    auto found = mFingers.find(touchInfo.mFingerId);
    if (found == mFingers.end())
      return false;
    found->second.mCurrentPoint = touchInfo.mCurrentPoint;

    if (mSwipeFingerId == touchInfo.mFingerId)
      addToSwipeQueue(touchInfo.mCurrentPoint, 0);


    auto foundControl0 = mFingers.find(mControlFingerIndexes[0]);
    auto foundControl1 = mFingers.find(mControlFingerIndexes[1]);
    if (foundControl0 == mFingers.end() || foundControl1 == mFingers.end()) {
      sendTouchInfo(touchInfo);
      updateDragDestination(touchInfo);
    }

    if (mSprite.multiTouchEnabled() && (touchInfo.mFingerId == foundControl0->second.mFingerId || touchInfo.mFingerId == foundControl1->second.mFingerId)) {
      Matrix44f parentTransform;
      parentTransform.setToIdentity();

      Sprite *currentParent = mSprite.getParent();
      while (currentParent) {
        parentTransform = currentParent->getInverseTransform() * parentTransform;
        currentParent = currentParent->getParent();
      }

      Vec3f fingerStart0 = foundControl0->second.mStartPoint;
      Vec3f fingerCurrent0 = foundControl0->second.mCurrentPoint;
      Vec3f fingerPositionOffset = (parentTransform * Vec4f(fingerCurrent0.x, fingerCurrent0.y, 0.0f, 1.0f) - parentTransform * Vec4f(fingerStart0.x, fingerStart0.y, 0.0f, 1.0f)).xyz();

      if (mFingers.size() > 1) {
        Vec3f fingerStart1 = foundControl1->second.mStartPoint;
        Vec3f fingerCurrent1 = foundControl1->second.mCurrentPoint;

        mStartDistance = fingerStart0.distance(fingerStart1);
        if (mStartDistance < mSpriteEngine.getMinTouchDistance())
          mStartDistance = mSpriteEngine.getMinTouchDistance();
        
        mCurrentDistance = fingerCurrent0.distance(fingerCurrent1);
        if (mCurrentDistance < mSpriteEngine.getMinTouchDistance())
          mCurrentDistance = mSpriteEngine.getMinTouchDistance();

        mCurrentScale = mCurrentDistance / mStartDistance;

        mCurrentAngle = atan2(fingerStart1.y - fingerStart0.y, fingerStart1.x - fingerStart0.x) -
                        atan2(fingerCurrent1.y - fingerCurrent0.y, fingerCurrent1.x - fingerCurrent0.x);

        if (mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_SCALE)) {
          mSprite.setScale(mStartScale*mCurrentScale);
        }

        if (mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_ROTATE)) {
          mSprite.setRotation(mStartRotation.z - mCurrentAngle * math::RADIAN2DEGREE);
        }
      }

      if (mSprite.multiTouchConstraintNotZero() && touchInfo.mFingerId == foundControl0->second.mFingerId) {
        Vec3f offset(0.0f, 0.0f, 0.0f);
        if (!mTappable && mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_POSITION_X))
          offset.x = fingerPositionOffset.x;
        if (!mTappable && mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_POSITION_Y))
          offset.y = fingerPositionOffset.y;
        mSprite.setPosition(mStartPosition + offset);
      }
    }

    sendTouchInfo(touchInfo);
    updateDragDestination(touchInfo);
  } else if (TouchInfo::Removed == touchInfo.mPhase) {
    sendTouchInfo(touchInfo);
    updateDragDestination(touchInfo);

    auto found = mFingers.find(touchInfo.mFingerId);
    if (found != mFingers.end())
      mFingers.erase(found);

    if (!mSprite.multiTouchEnabled())
      return true;

    if (mFingers.empty())
      resetTouchAnchor();
    else
      initializeTouchPoints();

    if (mFingers.empty() && swipeHappened()) {
      mSprite.swipe(mSwipeVector);
    }
  }

  return true;
}

void TouchProcess::update( const UpdateParams &updateParams )
{
  if (!mSprite.visible() || !mSprite.isEnabled() || !mOneTap || !mSprite.hasDoubleTap())
    return;
  if (mLastUpdateTime - mDoubleTapTime > mSpriteEngine.getDoubleTapTime()) {
    mSprite.tap(mFirstTapPos);
    mOneTap = false;
    mDoubleTapTime = mLastUpdateTime;
  }
}

void TouchProcess::sendTouchInfo( const TouchInfo &touchInfo )
{
  TouchInfo t = touchInfo;
  t.mCurrentAngle = mCurrentAngle;
  t.mCurrentScale = mCurrentScale;
  t.mCurrentDistance = mCurrentDistance;
  t.mStartDistance = mStartDistance;
  t.mNumberFingers = mFingers.size();

  if (touchInfo.mPhase == TouchInfo::Removed && t.mNumberFingers > 0)
    t.mNumberFingers = -1;

  auto found = mFingers.find(touchInfo.mFingerId);
  if (found != mFingers.end())
    t.mActive = found->second.mActive;

  mSprite.processTouchInfoCallback(touchInfo);
}

void TouchProcess::initializeFirstTouch()
{
  mFingers[mControlFingerIndexes[0]].mActive = true;
  mFingers[mControlFingerIndexes[0]].mStartPoint = mFingers[mControlFingerIndexes[0]].mCurrentPoint;
  mMultiTouchAnchor = mSprite.globalToLocal(mFingers[mControlFingerIndexes[0]].mStartPoint);
  mMultiTouchAnchor.x /= mSprite.getWidth();
  mMultiTouchAnchor.y /= mSprite.getHeight();
  mMultiTouchAnchor.z /= mSprite.getDepth();
  mStartAnchor = mSprite.getCenter();

  Vec3f positionOffset = mMultiTouchAnchor - mStartAnchor;
  positionOffset.x *= mSprite.getWidth();
  positionOffset.y *= mSprite.getHeight();
  positionOffset.x *= mSprite.getScale().x;
  positionOffset.y *= mSprite.getScale().y;
  positionOffset.rotate( Vec3f(0.0f, 0.0f, 1.0f), mSprite.getRotation().z * math::DEGREE2RADIAN);
  if (mSprite.multiTouchConstraintNotZero()) {
    mSprite.setCenter(mMultiTouchAnchor);
    mSprite.move(positionOffset);
  }

  mStartPosition = mSprite.getPosition();
}

void TouchProcess::initializeTouchPoints()
{
  if (!mSprite.multiTouchEnabled())
    return;

  if (mFingers.size() == 1) {
    mControlFingerIndexes[0] = mFingers.begin()->first;
    resetTouchAnchor();
    initializeFirstTouch();
    return;
  }

  int potentialFarthestIndexes[2];
  float potentialFarthestDistance = 0.0f;

  for ( auto it = mFingers.begin(), it2 = mFingers.end(); it != it2; ++it )
  {
    it->second.mActive = false;
  	for ( auto itt = mFingers.begin(), itt2 = mFingers.end(); itt != itt2; ++itt )
  	{
  		if (it == itt)
        continue;
      float newDistance = itt->second.mCurrentPoint.distance(it->second.mCurrentPoint);
      if (newDistance > potentialFarthestDistance) {
        potentialFarthestIndexes[0] = itt->first;
        potentialFarthestIndexes[1] = it->first;
        potentialFarthestDistance = newDistance;
      }
  	}
  }

  if (fabs(potentialFarthestDistance) < math::EPSILON)
    return;

  mControlFingerIndexes[0] = potentialFarthestIndexes[0];
  mControlFingerIndexes[1] = potentialFarthestIndexes[1];

  initializeFirstTouch();

  mFingers[mControlFingerIndexes[1]].mActive = true;
  mFingers[mControlFingerIndexes[1]].mStartPoint = mFingers[mControlFingerIndexes[1]].mCurrentPoint;
  mStartPosition = mSprite.getPosition();
  mStartRotation = mSprite.getRotation();
  mStartScale    = mSprite.getScale();
  mStartWidth    = mSprite.getWidth();
  mStartHeight   = mSprite.getHeight();
}

void TouchProcess::resetTouchAnchor()
{
  if (!mSprite.multiTouchEnabled())
    return;
  Vec3f positionOffset = mStartAnchor - mSprite.getCenter();
  positionOffset.x *= mSprite.getWidth();
  positionOffset.y *= mSprite.getHeight();
  positionOffset.x *= mSprite.getScale().x;
  positionOffset.y *= mSprite.getScale().y;
  positionOffset.rotate( Vec3f(0.0f, 0.0f, 1.0f), mSprite.getRotation().z * math::DEGREE2RADIAN);
  if (mSprite.multiTouchConstraintNotZero()) {
    mSprite.setCenter(mStartAnchor);
    mSprite.move(positionOffset);
  }
}

void TouchProcess::addToSwipeQueue( const Vec3f &currentPoint, int queueNum )
{
  SwipeQueueEvent swipeEvent;
  swipeEvent.mCurrentPoint = currentPoint;
  swipeEvent.mTimeStamp = mLastUpdateTime;
  mSwipeQueue.push_back(swipeEvent);
  if (mSwipeQueue.size() > mSpriteEngine.getSwipeQueueSize())
    mSwipeQueue.pop_front();
}

bool TouchProcess::swipeHappened()
{
  const float minSpeed = 800.0f;
  const float maxTimeThreshold = 0.5f;
  mSwipeVector = Vec3f();

  if (mSwipeQueue.size() < mSpriteEngine.getSwipeQueueSize())
    return false;

  for ( auto it = mSwipeQueue.begin(), it2 = mSwipeQueue.end(); it != it2 - 1; ++it ) {
  	mSwipeVector += (it+1)->mCurrentPoint - it->mCurrentPoint;
  }

  mSwipeVector /= static_cast<float>(mSwipeQueue.size() - 1);
  float averageDistance = mSwipeVector.distance(Vec3f());

  return (averageDistance >= minSpeed * 60.0f && (mLastUpdateTime - mSwipeQueue.front().mTimeStamp) < maxTimeThreshold);
}

void TouchProcess::updateDragDestination( const TouchInfo &touchInfo )
{
  auto found = mFingers.find(touchInfo.mFingerId);
  if (found == mFingers.end())
    return;


}

void TouchProcess::processTap( const TouchInfo &touchInfo )
{
  if (!mSprite.hasTap() && !mSprite.hasDoubleTap()) {
    mTappable = false;
    return;
  }

  if (touchInfo.mPhase == TouchInfo::Added && mFingers.empty()) {
    mTappable = true;
  } else if (mTappable) {
    if (mFingers.size() > 1 || (touchInfo.mPhase == TouchInfo::Moved && touchInfo.mCurrentPoint.distance(touchInfo.mStartPoint) > mSpriteEngine.getMinTapDistance())) {
      mTappable = false;
    } else if (touchInfo.mPhase == TouchInfo::Removed) {
      if (mSprite.hasTap() && !mSprite.hasDoubleTap()) {
        mSprite.tap(touchInfo.mCurrentPoint);
        mOneTap = false;
      } else if (mOneTap) {
        mSprite.doubleTap(touchInfo.mCurrentPoint);
        mOneTap = false;
      } else {
        mFirstTapPos = touchInfo.mCurrentPoint;
        mOneTap = true;
        mDoubleTapTime = mLastUpdateTime;
      }

      if (mFingers.size() == 1)
        mTappable = false;
    }
  }
}

}
}