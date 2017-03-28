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
	ci::vec3 mCurrentGlobalPoint;
	float     mTimeStamp;
};

class TouchProcess {
public:
	TouchProcess(SpriteEngine &, Sprite &sprite);
	~TouchProcess();

	bool					processTouchInfo(const TouchInfo &touchInfo);
	void					update(const UpdateParams &updateParams);

	bool					hasTouches() const;

	void					clearTouches();

private:
	void					sendTouchInfo(const TouchInfo &touchInfo);
	void					initializeFirstTouch();
	void					initializeTouchPoints();
	void					resetTouchAnchor();
    
	void					addToSwipeQueue(const ci::vec3 &currentPoint, int queueNum);
	bool					swipeHappened();

	void					updateDragDestination(const TouchInfo &touchInfo);
	int						getFingerIndex(int id);

	void					processTap(const TouchInfo &touchInfo);
	void					processTapInfo(const TouchInfo &touchInfo);
	void					sendTapInfo(const TapInfo::State, const int count, const ci::vec3& pt = ci::vec3(-1.0f, -1.0f, -1.0f));

	SpriteEngine&			mSpriteEngine;
	Sprite&					mSprite;

	std::map<int, TouchInfo> mFingers;
	std::list<int>			mFingerIndex;

	// the fingerIndexes of the current 2 control fingers
	int						mControlFingerIndexes[2];

	// the start point of the first finger, in local coordinates
	ci::vec3				mMultiTouchAnchor;

	// Start properties are stored to determine deltas
	ci::vec3				mStartPosition;

	// and to reset the anchor on touch completion and switches
	ci::vec3				mStartScale;
	ci::vec3				mStartRotation;
	ci::vec3				mStartAnchor;

	float					mStartWidth;
	float					mStartHeight;

	// These 4 variables are for calculating touch deltas.
	float					mStartDistance;
	float					mCurrentDistance;
	float					mCurrentScale;
	float					mCurrentAngle;

	ci::vec3				mSwipeVector;
	int						mSwipeFingerId;

	// the last few touch events and their time, for calculating swipes
	std::deque<SwipeQueueEvent> mSwipeQueue;

	// is the current finger action potentially a tap? If it is, the sprite won't move.
	bool					mTappable;

	// used to track for double taps
	bool					mOneTap;
	float					mDoubleTapTime;
	ci::vec3				mFirstTapPos;

	TapInfo					mTapInfo;

	float					mLastUpdateTime;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_PROCESS_H