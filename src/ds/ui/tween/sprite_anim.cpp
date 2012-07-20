#include "ds/ui/tween/sprite_anim.h"

#include "ds/ui/sprite/sprite.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::SpriteAnimatable
 */
SpriteAnimatable::SpriteAnimatable()
{
}

SpriteAnimatable::~SpriteAnimatable()
{
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_POSITION()
{
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimPosition; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return s.getPosition(); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setPosition(v); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SCALE()
{
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimScale; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return s.getScale(); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setScale(v); });
  return ANIM;
}

void SpriteAnimatable::animStop()
{
  mAnimPosition.stop();
  mAnimScale.stop();
}

} // namespace ui
} // namespace ds
