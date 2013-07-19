#include "touch_process.h"
#include "ds/math/math_defs.h"
#include "ds/ui/sprite/sprite.h"
#include "multi_touch_constraints.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "drag_destination_info.h"

using namespace ci;

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
	std::vector<int> fingers;
	for ( auto i = mFingers.begin(), e = mFingers.end(); i != e; ++i )
	{
	fingers.push_back(i->first);
	}
	mSpriteEngine.clearFingers(fingers);

	mFingers.clear();
}

bool TouchProcess::processTouchInfo( const TouchInfo &touchInfo )
{
	if (!mSprite.visible() || !mSprite.isEnabled())
		return false;

	mSprite.userInputReceived();

	processTap(touchInfo);
	processTapInfo(touchInfo);

	if (TouchInfo::Added == touchInfo.mPhase) {
		mFingers[touchInfo.mFingerId] = touchInfo;
		mFingerIndex.push_back(touchInfo.mFingerId);

		if (mFingers.size() == 1) {
			mSwipeQueue.clear();
			mSwipeFingerId = touchInfo.mFingerId;
			addToSwipeQueue(touchInfo.mCurrentGlobalPoint, 0);
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
		found->second.mCurrentGlobalPoint = touchInfo.mCurrentGlobalPoint;

		if (mSwipeFingerId == touchInfo.mFingerId)
			addToSwipeQueue(touchInfo.mCurrentGlobalPoint, 0);


		auto foundControl0 = mFingers.find(mControlFingerIndexes[0]);
		auto foundControl1 = mFingers.find(mControlFingerIndexes[1]);

		if (mSprite.multiTouchEnabled() && (touchInfo.mFingerId == foundControl0->second.mFingerId || touchInfo.mFingerId == foundControl1->second.mFingerId)) {
			Matrix44f parentTransform;
			parentTransform.setToIdentity();

			Sprite *currentParent = mSprite.getParent();
			while (currentParent) {
				parentTransform = currentParent->getInverseTransform() * parentTransform;
				currentParent = currentParent->getParent();
			}

			Vec3f fingerStart0 = foundControl0->second.mStartPoint;
			Vec3f fingerCurrent0 = foundControl0->second.mCurrentGlobalPoint;
			Vec3f fingerPositionOffset = (parentTransform * Vec4f(fingerCurrent0.x, fingerCurrent0.y, 0.0f, 1.0f) - parentTransform * Vec4f(fingerStart0.x, fingerStart0.y, 0.0f, 1.0f)).xyz();

			if (mFingers.size() > 1) {
				Vec3f fingerStart1 = foundControl1->second.mStartPoint;
				Vec3f fingerCurrent1 = foundControl1->second.mCurrentGlobalPoint;

				mStartDistance = fingerStart0.distance(fingerStart1);
				if (mStartDistance < mSpriteEngine.getMinTouchDistance()){
					mStartDistance = mSpriteEngine.getMinTouchDistance();
				}

				mCurrentDistance = fingerCurrent0.distance(fingerCurrent1);
				if (mCurrentDistance < mSpriteEngine.getMinTouchDistance()){
					mCurrentDistance = mSpriteEngine.getMinTouchDistance();
				}

				mCurrentScale = mCurrentDistance / mStartDistance;

				mCurrentAngle = atan2(fingerStart1.y - fingerStart0.y, fingerStart1.x - fingerStart0.x) -
					atan2(fingerCurrent1.y - fingerCurrent0.y, fingerCurrent1.x - fingerCurrent0.x);

				if (mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_SCALE)) {
					if(mSprite.mTouchScaleSizeMode){
						mSprite.setSize(mStartWidth * mCurrentScale, mStartHeight * mCurrentScale);
					} else {
						mSprite.setScale(mStartScale*mCurrentScale);
					}
				}

				if (mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_ROTATE)) {
					mSprite.setRotation(mStartRotation.z - mCurrentAngle * math::RADIAN2DEGREE);
				}
			}

			if (mSprite.mMultiTouchConstraints != ds::ui::MULTITOUCH_INFO_ONLY && touchInfo.mFingerId == foundControl0->second.mFingerId) {
				Vec3f offset(0.0f, 0.0f, 0.0f);

				if (!mTappable && mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_POSITION_X)) {
					offset.x = fingerPositionOffset.x;
				}
				
				if (!mTappable && mSprite.hasMultiTouchConstraint(MULTITOUCH_CAN_POSITION_Y)) {
					offset.y = fingerPositionOffset.y;
				}

				mSprite.setPosition(mStartPosition + offset);
			}
		}

		sendTouchInfo(touchInfo);
		updateDragDestination(touchInfo);

	} else if (TouchInfo::Removed == touchInfo.mPhase) {
		sendTouchInfo(touchInfo);
		updateDragDestination(touchInfo);

		auto found = mFingers.find(touchInfo.mFingerId);
		if (found != mFingers.end()){
			mFingers.erase(found);
		}

		mFingerIndex.remove(touchInfo.mFingerId);

		if (!mSprite.multiTouchEnabled()){
			return true;
		}

		if (mFingers.empty()) {
			resetTouchAnchor();
		} else {
			initializeTouchPoints();
		}

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
	t.mNumberFingers -= 1;

	t.mFingerIndex = getFingerIndex(touchInfo.mFingerId);

	auto found = mFingers.find(touchInfo.mFingerId);
	if (found != mFingers.end()){
	t.mActive = found->second.mActive;
	}

	mSprite.processTouchInfoCallback(t);
}

void TouchProcess::initializeFirstTouch()
{
	mFingers[mControlFingerIndexes[0]].mActive = true;
	mFingers[mControlFingerIndexes[0]].mStartPoint = mFingers[mControlFingerIndexes[0]].mCurrentGlobalPoint;
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
	if (mSprite.mMultiTouchConstraints != ds::ui::MULTITOUCH_INFO_ONLY) {
		mSprite.setCenter(mMultiTouchAnchor);
		mSprite.move(positionOffset);
	}

	mStartPosition = mSprite.getPosition();
}

void TouchProcess::initializeTouchPoints()
{
	if (!mSprite.multiTouchEnabled()){
		return;
	}

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
			float newDistance = itt->second.mCurrentGlobalPoint.distance(it->second.mCurrentGlobalPoint);
			if (newDistance > potentialFarthestDistance) {
				potentialFarthestIndexes[0] = itt->first;
				potentialFarthestIndexes[1] = it->first;
				potentialFarthestDistance = newDistance;
			}
		}
	}

	if (fabs(potentialFarthestDistance) < math::EPSILON)
		return;

	resetTouchAnchor();

	mControlFingerIndexes[0] = potentialFarthestIndexes[0];
	mControlFingerIndexes[1] = potentialFarthestIndexes[1];

	initializeFirstTouch();

	mFingers[mControlFingerIndexes[1]].mActive = true;
	mFingers[mControlFingerIndexes[1]].mStartPoint = mFingers[mControlFingerIndexes[1]].mCurrentGlobalPoint;
	mStartPosition = mSprite.getPosition();
	mStartRotation = mSprite.getRotation();
	mStartScale    = mSprite.getScale();
	mStartWidth    = mSprite.getWidth();
	mStartHeight   = mSprite.getHeight();
}

void TouchProcess::resetTouchAnchor()
{
	if (!mSprite.multiTouchEnabled()){
		return;
	}

	Vec3f positionOffset = mStartAnchor - mSprite.getCenter();
	positionOffset.x *= mSprite.getWidth();
	positionOffset.y *= mSprite.getHeight();
	positionOffset.x *= mSprite.getScale().x;
	positionOffset.y *= mSprite.getScale().y;
	positionOffset.rotate( Vec3f(0.0f, 0.0f, 1.0f), mSprite.getRotation().z * math::DEGREE2RADIAN);
	if (mSprite.mMultiTouchConstraints != ds::ui::MULTITOUCH_INFO_ONLY) {
		mSprite.setCenter(mStartAnchor);
		mSprite.move(positionOffset);
	}
}

void TouchProcess::addToSwipeQueue( const Vec3f &currentPoint, int queueNum )
{
	SwipeQueueEvent swipeEvent;
	swipeEvent.mCurrentGlobalPoint = currentPoint;
	swipeEvent.mTimeStamp = mLastUpdateTime;
	mSwipeQueue.push_back(swipeEvent);
	if (mSwipeQueue.size() > mSpriteEngine.getSwipeQueueSize()){
		mSwipeQueue.pop_front();
	}
}

bool TouchProcess::swipeHappened()
{
	const float minSpeed = 800;
	const float maxTimeThreshold = 0.5f;
	mSwipeVector = Vec3f();

	if (mSwipeQueue.size() < mSpriteEngine.getSwipeQueueSize()){
		return false;
	}

	for ( auto it = mSwipeQueue.begin(), it2 = mSwipeQueue.end(); it != it2 - 1; ++it ) {
		mSwipeVector += (it+1)->mCurrentGlobalPoint - it->mCurrentGlobalPoint;
	}

	mSwipeVector /= static_cast<float>(mSwipeQueue.size() - 1);
	float averageDistance = mSwipeVector.distance(Vec3f());

	return (averageDistance >= minSpeed * 0.016f && (mLastUpdateTime - mSwipeQueue.front().mTimeStamp) < maxTimeThreshold);
}

void TouchProcess::updateDragDestination( const TouchInfo &touchInfo )
{
	auto found = mFingers.find(touchInfo.mFingerId);
	if (found == mFingers.end()){
		return;
	}

	Sprite *dragDestinationSprite = mSpriteEngine.getDragDestinationSprite(touchInfo.mCurrentGlobalPoint, &mSprite);

	if (!dragDestinationSprite)
		return;

	DragDestinationInfo dragInfo = {touchInfo.mCurrentGlobalPoint, DragDestinationInfo::Null, &mSprite};

	if (!mSprite.getDragDestination() && dragDestinationSprite) {
		dragInfo.mPhase = DragDestinationInfo::Entered;
		mSprite.setDragDestination(dragDestinationSprite);
	} else if (false) {
		dragInfo.mPhase = DragDestinationInfo::Updated;
	} else if (false) {
		dragInfo.mPhase = DragDestinationInfo::Exited;
	}

	if (mSprite.getDragDestination() && mFingers.size() < 2 && touchInfo.mPhase == TouchInfo::Removed) {
		dragInfo.mPhase = DragDestinationInfo::Released;
	}

	mSprite.dragDestination(mSprite.getDragDestination(), dragInfo);
	if (mSprite.getDragDestination()){
		mSprite.getDragDestination()->dragDestination(mSprite.getDragDestination(), dragInfo);
	}

	if (dragInfo.mPhase == DragDestinationInfo::Released || dragInfo.mPhase == DragDestinationInfo::Exited) {
		mSprite.setDragDestination(nullptr);
	}
}

void TouchProcess::processTap( const TouchInfo &touchInfo )
{
	if (mSprite.hasTapInfo()) return;
	if (!mSprite.hasTap() && !mSprite.hasDoubleTap()) {
		mTappable = false;
		return;
	}

	if (touchInfo.mPhase == TouchInfo::Added && mFingers.empty()) {
		mTappable = true;
	} else if (mTappable) {
		if (mFingers.size() > 1 || (touchInfo.mPhase == TouchInfo::Moved && touchInfo.mCurrentGlobalPoint.distance(touchInfo.mStartPoint) > mSpriteEngine.getMinTapDistance())) {
			mTappable = false;
		} else if (touchInfo.mPhase == TouchInfo::Removed) {
			if (mSprite.hasTap() && !mSprite.hasDoubleTap()) {
				mSprite.tap(touchInfo.mCurrentGlobalPoint);
				mOneTap = false;
			} else if (mOneTap) {
				mSprite.doubleTap(touchInfo.mCurrentGlobalPoint);
				mOneTap = false;
			} else {
				mFirstTapPos = touchInfo.mCurrentGlobalPoint;
				mOneTap = true;
				mDoubleTapTime = mLastUpdateTime;
			}

			if (mFingers.size() == 1){
				mTappable = false;
			}
		}
	}
}

void TouchProcess::processTapInfo( const TouchInfo &touchInfo )
{
	if (mSprite.hasTap() || mSprite.hasDoubleTap()) return;
	if (!mSprite.hasTapInfo()) {
		mTappable = false;
		return;
	}

	// NOTE: This currently doesn't work with double taps.  Don't
	// need it right now and wasn't planning on needing to do this
	// at all.
	if (touchInfo.mPhase == TouchInfo::Added && mFingers.empty()) {
		mTappable = true;
		sendTapInfo(TapInfo::Waiting, 0);
	} else if (mTappable) {
		// User cancelled at some point
		if (mTapInfo.mState == TapInfo::Done) {
			mTappable = false;
			return;
		}
		if (mFingers.size() > 1 || (touchInfo.mPhase == TouchInfo::Moved && touchInfo.mCurrentGlobalPoint.distance(touchInfo.mStartPoint) > mSpriteEngine.getMinTapDistance())) {
			mTappable = false;
			sendTapInfo(TapInfo::Done, 0);
		} else if (touchInfo.mPhase == TouchInfo::Removed) {
			if (mSprite.hasTapInfo()) {
			sendTapInfo(TapInfo::Tapped, 1, touchInfo.mCurrentGlobalPoint);
			mTappable = false;
			mTapInfo.mCount = 0;
			mTapInfo.mState = TapInfo::Done;
			}

			if (mFingers.size() == 1)
			mTappable = false;
		}
	}
}

void TouchProcess::sendTapInfo(const TapInfo::State s, const int count, const ci::Vec3f& pt)
{
	mTapInfo.mState = s;
	mTapInfo.mCount = count;
	mTapInfo.mCurrentGlobalPoint = pt;
	if (!mSprite.tapInfo(mTapInfo)) {
		mTapInfo.mState = TapInfo::Done;
	}
}

int TouchProcess::getFingerIndex( int id )
{
	int i = 0;
	for (auto it = mFingerIndex.begin(), it2 = mFingerIndex.end(); it != it2; ++it, ++i) {
		if (*it == id){
			return i;
		}
	}
	return -1;
}

}
}