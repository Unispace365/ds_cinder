#pragma once
#ifndef DS_UI_TOUCH_PROCESS_H
#define DS_UI_TOUCH_PROCESS_H

#include <map>
#include <deque>
#include <list>
#include "touch_info.h"
#include "ds/params/update_params.h"
#include "ds/ui/touch/tap_info.h"
#include "cinder/Vector.h"

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;

struct SwipeQueueEvent {
	ci::Vec3f mCurrentGlobalPoint;
	float     mTimeStamp;
};

class TouchProcess {
public:
	TouchProcess(SpriteEngine &, Sprite &sprite);
	~TouchProcess();

	bool					processTouchInfo(const TouchInfo &touchInfo);
	void					update(const UpdateParams &updateParams);

	bool					hasTouches() const;

private:
	void					sendTouchInfo(const TouchInfo &touchInfo);
	void					initializeFirstTouch();
	void					initializeTouchPoints();
	void					resetTouchAnchor();
    
	void					addToSwipeQueue(const ci::Vec3f &currentPoint, int queueNum);
	bool					swipeHappened();

	void					updateDragDestination(const TouchInfo &touchInfo);
	int						getFingerIndex(int id);

	void					processTap(const TouchInfo &touchInfo);
	void					processTapInfo(const TouchInfo &touchInfo);
	void					sendTapInfo(const TapInfo::State, const int count, const ci::Vec3f& pt = ci::Vec3f(-1.0f, -1.0f, -1.0f));

	SpriteEngine&			mSpriteEngine;
	Sprite&					mSprite;

	std::map<int, TouchInfo> mFingers;
	std::list<int>			mFingerIndex;

	// the fingerIndexes of the current 2 control fingers
	int						mControlFingerIndexes[2];

	// the start point of the first finger, in local coordinates
	ci::Vec3f				mMultiTouchAnchor;

	// Start properties are stored to determine deltas
	ci::Vec3f				mStartPosition;

	// and to reset the anchor on touch completion and switches
	ci::Vec3f				mStartScale;
	ci::Vec3f				mStartRotation;
	ci::Vec3f				mStartAnchor;

	float					mStartWidth;
	float					mStartHeight;

	// These 4 variables are for calculating touch deltas.
	float					mStartDistance;
	float					mCurrentDistance;
	float					mCurrentScale;
	float					mCurrentAngle;

	ci::Vec3f				mSwipeVector;
	int						mSwipeFingerId;

	// the last few touch events and their time, for calculating swipes
	std::deque<SwipeQueueEvent> mSwipeQueue;

	// is the current finger action potentially a tap? If it is, the sprite won't move.
	bool					mTappable;

	// used to track for double taps
	bool					mOneTap;
	float					mDoubleTapTime;
	ci::Vec3f				mFirstTapPos;

	TapInfo					mTapInfo;

	float					mLastUpdateTime;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_PROCESS_H