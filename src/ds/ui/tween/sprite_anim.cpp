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

const SpriteAnim<ci::Color>& SpriteAnimatable::ANIM_COLOR()
{
  static ds::ui::SpriteAnim<ci::Color>  ANIM(
    [](ds::ui::Sprite& s)->ci::Anim<ci::Color>& { return s.mAnimColor; },
    [](ds::ui::Sprite& s)->ci::Color { return s.getColor(); },
    [](const ci::Color& v, ds::ui::Sprite& s) { s.setColor(v); });
  return ANIM;
}

const SpriteAnim<float>& SpriteAnimatable::ANIM_OPACITY()
{
  static ds::ui::SpriteAnim<float>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<float>& { return s.mAnimOpacity; },
          [](ds::ui::Sprite& s)->float { return s.getOpacity(); },
          [](const float& v, ds::ui::Sprite& s) { s.setOpacity(v); });
  return ANIM;
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

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SIZE()
{
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimSize; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return ci::Vec3f(s.getWidth(), s.getHeight(), s.getDepth()); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setSizeAll(v.x, v.y, v.z); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_ROTATION()
{
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
    [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimRotation; },
    [](ds::ui::Sprite& s)->ci::Vec3f { return s.getRotation(); },
    [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setRotation(v); });
  return ANIM;
}

void SpriteAnimatable::animStop()
{
  mAnimColor.stop();
  mAnimOpacity.stop();
  mAnimPosition.stop();
  mAnimScale.stop();
  mAnimSize.stop();
}

} // namespace ui
} // namespace ds
