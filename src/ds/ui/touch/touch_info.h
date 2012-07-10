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
  glm::vec2 mStartPoint;
  glm::vec2 mCurrentPoint;
  glm::vec2 mDeltaPoint;
  Sprite   *mPickedSprite;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_INFO_H
