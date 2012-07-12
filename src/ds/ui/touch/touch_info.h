#pragma once
#ifndef DS_UI_TOUCH_INFO_H
#define DS_UI_TOUCH_INFO_H
#include "cinder/Vector.h"

using namespace ci;

namespace ds {
namespace ui {

class Sprite;

struct TouchInfo
{
  enum Phase
  {
    Added,
    Moved,
    Removed
  };

  int       mFingerIndex;
  int       mNumberFingers;
  int       mFingerId;
  Phase     mPhase;
  Vec2f     mStartPoint;
  Vec2f     mCurrentPoint;
  Vec2f     mDeltaPoint;
  Sprite   *mPickedSprite;
  bool      mActive;
  float     mStartDistance;
  float     mCurrentDistance;
  float     mCurrentScale;
  float     mCurrentAngle;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_INFO_H
