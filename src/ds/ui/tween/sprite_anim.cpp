#include "ds/ui/tween/sprite_anim.h"

#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/tween/tweenline.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::SpriteAnimatable
 */
SpriteAnimatable::SpriteAnimatable(Sprite& s, SpriteEngine& e)
		: mOwner(s)
		, mEngine(e) {
}

SpriteAnimatable::~SpriteAnimatable() {
}

const SpriteAnim<ci::Color>& SpriteAnimatable::ANIM_COLOR() {
  static ds::ui::SpriteAnim<ci::Color>  ANIM(
    [](ds::ui::Sprite& s)->ci::Anim<ci::Color>& { return s.mAnimColor; },
    [](ds::ui::Sprite& s)->ci::Color { return s.getColor(); },
    [](const ci::Color& v, ds::ui::Sprite& s) { s.setColor(v); });
  return ANIM;
}

const SpriteAnim<float>& SpriteAnimatable::ANIM_OPACITY() {
  static ds::ui::SpriteAnim<float>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<float>& { return s.mAnimOpacity; },
          [](ds::ui::Sprite& s)->float { return s.getOpacity(); },
          [](const float& v, ds::ui::Sprite& s) { s.setOpacity(v); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_POSITION() {
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimPosition; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return s.getPosition(); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setPosition(v); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SCALE() {
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimScale; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return s.getScale(); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setScale(v); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SIZE() {
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
          [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimSize; },
          [](ds::ui::Sprite& s)->ci::Vec3f { return ci::Vec3f(s.getWidth(), s.getHeight(), s.getDepth()); },
          [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setSizeAll(v.x, v.y, v.z); });
  return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_ROTATION() {
  static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
    [](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimRotation; },
    [](ds::ui::Sprite& s)->ci::Vec3f { return s.getRotation(); },
    [](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setRotation(v); });
  return ANIM;
}

void SpriteAnimatable::tweenColor(	const ci::Color& c, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimColor.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_COLOR(), c, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenOpacity(const float opacity, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimOpacity.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_OPACITY(), opacity, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenPosition(const ci::Vec3f& pos, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimPosition.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_POSITION(), pos, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenRotation(const ci::Vec3f& rot, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimRotation.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_ROTATION(), rot, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenScale(	const ci::Vec3f& scale, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimScale.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_SCALE(), scale, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenSize(	const ci::Vec3f& size, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimSize.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_SIZE(), size, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::animStop() {
	mAnimColor.stop();
	mAnimOpacity.stop();
	mAnimPosition.stop();
	mAnimScale.stop();
	mAnimSize.stop();
}

} // namespace ui
} // namespace ds
