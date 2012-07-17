#pragma once
#ifndef DS_UI_TOUCH_MANAGER_H
#define DS_UI_TOUCH_MANAGER_H
#include <map>
#include "cinder/app/TouchEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Color.h"

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

    void                        mouseTouchBegin( MouseEvent event, int id );
    void                        mouseTouchMoved( MouseEvent event, int id );
    void                        mouseTouchEnded( MouseEvent event, int id );

    void                        touchesBegin( TouchEvent event );
    void                        touchesMoved( TouchEvent event );
    void                        touchesEnded( TouchEvent event );

    void                        drawTouches() const;
    void                        setTouchColor(const ci::Color &color);
  private:
    Engine &mEngine;

    std::map<int, ui::Sprite *> mFingerDispatcher;
    std::map<int, Vec3f>        mTouchStartPoint;
    std::map<int, Vec3f>        mTouchPreviousPoint;
    ci::Color                   mTouchColor;
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TOUCH_MANAGER_H
