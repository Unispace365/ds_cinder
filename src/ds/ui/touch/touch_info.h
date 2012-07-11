#pragma once
#ifndef DS_UI_TOUCH_INFO_H
#define DS_UI_TOUCH_INFO_H

namespace ds {
namespace ui {

struct TouchInfo
{
  enum Phase
  {
    Added,
    Moved,
    Removed
  };

  int       mFingerId;
  Phase     mPhase;
  Vec2f     mStartPoint;
  Vec2f     mCurrentPoint;
  Vec2f     mDeltaPoint;
  Sprite   *mPickedSprite;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_INFO_H
