#pragma once
#ifndef DS_UI_TWEEN_TWEENLINE_H_
#define DS_UI_TWEEN_TWEENLINE_H_

#include <cinder/Timeline.h>
#include "ds/ui/tween/sprite_anim.h"

// I REALLY don't want this here but it's sorta required for the tweenLerp to work
using namespace ci;

namespace ds {
namespace ui {

/**
 * \class ds::ui::Tweenline
 * A wrapper around the Cinder timeline that provides some sprite-based management.
 */
class Tweenline {
  public:
    Tweenline(cinder::Timeline&);

    template <typename T>
    void                  apply( Sprite&, const SpriteAnim<T>&, const T& end,
                                 float duration, ci::EaseFn easeFunction = ci::easeNone,
                                 typename ci::Tween<T>::LerpFn lerpFunction = &tweenLerp<T>);

    // Clients can go nuts with full access to the cinder timeline
    cinder::Timeline&     getTimeline();

  private:
    Tweenline();
    cinder::Timeline&     mTimeline;
};

template <typename T>
void Tweenline::apply( Sprite& s, const SpriteAnim<T>& a, const T& end,
                       float duration, ci::EaseFn easeFunction,
                       typename ci::Tween<T>::LerpFn lerpFunction)
{
  auto&   anim = a.getAnim(s); 
  auto    ans = mTimeline.apply(&anim, a.getStartValue(s), end, duration, easeFunction, lerpFunction);
  ds::ui::Sprite*           s_ptr = &s;
  const ci::Vec3f*          vec_ptr = anim.ptr();
  auto                      assignF = a.getAssignValue();
  ans.updateFn([s_ptr, vec_ptr, assignF](){ assignF(*vec_ptr, *s_ptr);});
}

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_TWEENLINE_H_