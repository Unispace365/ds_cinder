#pragma once
#ifndef DS_UI_TOUCH_PROCESS_H
#define DS_UI_TOUCH_PROCESS_H

#include "touch_info.h"
#include "ds/params/update_params.h"
#include "cinder/Vector.h"
#include <map>
#include <deque>

using namespace ci;

namespace ds {
namespace ui {

class Sprite;
class SpriteEngine;

struct SwipeQueueEvent
{
  Vec2f mCurrentPoint;
  float mTimeStamp;
};

class TouchProcess
{
  public:
    TouchProcess(SpriteEngine &, Sprite &sprite);
    ~TouchProcess();

    bool                    processTouchInfo(const TouchInfo &touchInfo);
    void                    update(const UpdateParams &updateParams);
  private:
    SpriteEngine            &mSpriteEngine;
    Sprite                  &mSprite;

    void                     sendTouchInfo(const TouchInfo &touchInfo);
    void                     initializeFirstTouch();
    void                     initializeTouchPoints();
    void                     resetTouchAnchor();
    
    void                     addToSwipeQueue(const Vec2f &currentPoint, int queueNum);
    bool                     swipeHappened();

    void                     updateDragDestination(const TouchInfo &touchInfo);

    std::map<int, TouchInfo> mFingers;

    // the fingerIndexes of the current 2 control fingers
    int                      mControlFingerIndexes[2];

    // the start point of the first finger, in local coordinates
    Vec2f                    mMultiTouchAnchor;

    // Start properties are stored to determine deltas
    Vec2f                    mStartPosition;

    // and to reset the anchor on touch completion and switches
    Vec2f                    mStartScale;
    float                    mStartRotation;
    Vec2f                    mStartAnchor;

    float                    mStartWidth;
    float                    mStartHeight;

    // These 4 variables are for calculating touch deltas.
    float                    mStartDistance;
    float                    mCurrentDistance;
    float                    mCurrentScale;
    float                    mCurrentAngle;

    Vec2f                    mSwipeVector;
    int                      mSwipeFingerId;

    // the last few touch events and their time, for calculating swipes
    std::deque<SwipeQueueEvent> mSwipeQueue;

    void                     processTap(const TouchInfo &touchInfo);

    // is the current finger action potentially a tap? If it is, the sprite won't move.
    bool                     mTappable;

    // used to track for double taps
    bool                     mOneTap;
    float                    mDoubleTapTime;
    Vec2f                    mFirstTapPos;

    float                    mLastUpdateTime;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_PROCESS_H