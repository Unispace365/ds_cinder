#pragma once
#ifndef DS_UI_TWEEN_TWEENLINE_H_
#define DS_UI_TWEEN_TWEENLINE_H_

#include <cinder/Timeline.h>
#include "ds/ui/tween/sprite_anim.h"

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
	void                  apply(Sprite&, const SpriteAnim<T>&, const T& end,
								float duration, ci::EaseFn easeFunction = ci::easeNone,
								const std::function<void(void)>& finishFn = nullptr,
								const float delay = 0,
								const std::function<void(void)>& updateFn = nullptr);

    // Clients can go nuts with full access to the cinder timeline
    cinder::Timeline&     getTimeline();

  private:
    Tweenline();
    cinder::Timeline&     mTimeline;
};

template <typename T>
void Tweenline::apply(Sprite& s, const SpriteAnim<T>& a, const T& end,
                       float duration, ci::EaseFn easeFunction,
                       const std::function<void(void)>& finishFn,
					   const float delay,
					   const std::function<void(void)>& updateFn)
{
  auto&   anim = a.getAnim(s); 

  auto    ans = mTimeline.apply(&anim, a.getStartValue(s), end, duration, easeFunction);
  ds::ui::Sprite*           s_ptr = &s;
  const T*                  value_ptr = anim.ptr();
  auto                      assignF = a.getAssignValue();
  ans.updateFn([s_ptr, value_ptr, assignF, updateFn](){ assignF(*value_ptr, *s_ptr); if(updateFn) updateFn(); });
  if (finishFn) ans.finishFn(finishFn);
  ans.delay(delay);
}

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_TWEENLINE_H_