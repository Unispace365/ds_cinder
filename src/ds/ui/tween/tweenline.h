#pragma once
#ifndef DS_UI_TWEEN_TWEENLINE_H_
#define DS_UI_TWEEN_TWEENLINE_H_

#include <unordered_map>
#include <cinder/Tween.h>
#include <cinder/Vector.h>
#include "ds/app/app_defs.h"

namespace ds {
namespace ui {
class Sprite;

/**
 * \class ds::ui::Tweenline
 * A wrapper around the Cinder timeline that provides some sprite-based management.
 */
class Tweenline {
  public:
    // Common tweening targets
    typedef std::function<void(Sprite&, const ci::Vec3f&)>  TWEEN_VEC3F;

    static const TWEEN_VEC3F&     POSITION();

  public:
    Tweenline(cinder::Timeline&);

    bool                  addVec3f( Sprite&, const ci::Vec3f& start, const ci::Vec3f& end,
                                    const std::function<void(Sprite&, const ci::Vec3f&)>&,
                                    float duration, ci::EaseFn easeFunction = ci::easeNone);

  private:
    Tweenline();
    cinder::Timeline&     mTimeline;

    // Maintain tokens for all tweening objects, so I can cancel them
    typedef cinder::Anim<ci::Vec3f>
                          AnimVec3f;

    std::unordered_map<ds::sprite_id_t, std::vector<std::unique_ptr<AnimVec3f>>>
                          mVec3f;

    void                  removeVec3f(const ds::sprite_id_t, const ci::Vec3f*);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_TWEENLINE_H_