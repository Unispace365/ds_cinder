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
                                 const std::function<void(void)>& finishFn = nullptr,
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
                       const std::function<void(void)>& finishFn,
                       typename ci::Tween<T>::LerpFn lerpFunction)
{
  auto&   anim = a.getAnim(s); 
  // OK, for some reason, this broke once I added float<> anims.  What's
  // going on is that, no matter what, the lerp is always a ci::Vec3f type,
  // probably not coincidentally the only type I was using before adding floats.
  // This doesn't affect the default lerp if I don't pass mine in, not sure
  // what the deal is.
//  auto    ans = mTimeline.apply(&anim, a.getStartValue(s), end, duration, easeFunction, lerpFunction);
  auto    ans = mTimeline.apply(&anim, a.getStartValue(s), end, duration, easeFunction);
  ds::ui::Sprite*           s_ptr = &s;
  const T*                  value_ptr = anim.ptr();
  auto                      assignF = a.getAssignValue();
  ans.updateFn([s_ptr, value_ptr, assignF](){ assignF(*value_ptr, *s_ptr);});
  if (finishFn) ans.finishFn(finishFn);
}

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_TWEENLINE_H_