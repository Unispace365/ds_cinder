#include "ds/ui/tween/tweenline.h"

#include <cinder/Timeline.h>
#include "ds/ui/sprite/sprite.h"

namespace ds {
namespace ui {

const Tweenline::TWEEN_VEC3F& Tweenline::POSITION()
{
  static TWEEN_VEC3F    ANS = [](ds::ui::Sprite& s, const ci::Vec3f& v){s.setPosition(v);};
  return ANS;
}

/**
 * \class ds::ui::Tweenline
 */
Tweenline::Tweenline(cinder::Timeline& tl)
  : mTimeline(tl)
{
}

bool Tweenline::addVec3f( Sprite& s, const ci::Vec3f& start, const ci::Vec3f& end,
                          const std::function<void(Sprite&, const ci::Vec3f&)>& f,
                          float duration, ci::EaseFn easeFunction)
{
  const ds::sprite_id_t     id = s.getId();
  if (id == ds::EMPTY_SPRITE_ID) return false;
  // Make sure the animation is fully allocated and in my cache before I
  // place it in the timeline
  std::unique_ptr<AnimVec3f>  animUp(new AnimVec3f());
  if (!animUp) return false;

  try {
    if (mVec3f.empty() || mVec3f.find(id) == mVec3f.end()) {
      mVec3f[id] = std::vector<std::unique_ptr<AnimVec3f>>();
    }
    auto                      it = mVec3f.find(id);
    it->second.push_back(std::move(animUp));
    AnimVec3f*                anim = it->second.back().get();

  	auto ans = mTimeline.apply(anim, start, end, duration, easeFunction);
    ds::ui::Sprite*           s_ptr = &s;
    const ci::Vec3f*          vec_ptr = anim->ptr();
    ans.updateFn([s_ptr, vec_ptr, f](){ f(*s_ptr, *vec_ptr);});
    ans.finishFn([this, id, vec_ptr](){this->removeVec3f(id, vec_ptr);});

    return true;
  } catch (std::exception const&) {
  }
  return false;
}

void Tweenline::removeVec3f(const ds::sprite_id_t id, const ci::Vec3f* vec)
{
  try {
    if (mVec3f.empty()) return;
    auto it = mVec3f.find(id);
    if (it == mVec3f.end()) return;

    for (auto vit=it->second.begin(), vend=it->second.end(); vit != vend; ++vit) {
      AnimVec3f*    anim = vit->get();
      if (anim && anim->ptr() == vec) {
        anim->stop();
        it->second.erase(vit);
        return;
      }
    }
  } catch (std::exception const&) {
  }
}

} // namespace ui
} // namespace ds
