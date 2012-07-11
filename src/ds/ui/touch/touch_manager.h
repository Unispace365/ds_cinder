#pragma once
#ifndef DS_UI_TOUCH_MANAGER_H
#define DS_UI_TOUCH_MANAGER_H
#include <map>
#include "cinder/app/TouchEvent.h"

using namespace ci;
using namespace ci::app;

namespace ds {

class Engine;

namespace ui {

class Sprite;

class TouchManager
{
  public:
    TouchManager(Engine &engine);

    void                        touchesBegin( TouchEvent event );
    void                        touchesMoved( TouchEvent event );
    void                        touchesEnded( TouchEvent event );
  private:
    Engine &mEngine;

    std::map<int, ui::Sprite *> mFingerDispatcher;
    std::map<int, Vec2f>        mTouchStartPoint;
    std::map<int, Vec2f>        mTouchPreviousPoint;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_MANAGER_H
