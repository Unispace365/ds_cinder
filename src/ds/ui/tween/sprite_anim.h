#pragma once
#ifndef DS_UI_TWEEN_SPRITEANIM_H_
#define DS_UI_TWEEN_SPRITEANIM_H_

#include <cinder/Tween.h>
#include <cinder/Vector.h>

namespace ds {
namespace ui {
class Sprite;

/**
 * \class ds::ui::SpriteAnim
 * A utility to class to provide animation access to a single
 * sprite property.
 *
 * To construct a sprite anim, you first need a property to
 * animate (generally a ci::Anim<> that exists in the sprite),
 * then you need to make this class and give access to:
 * 1. The property being animated
 * 2. A getter on the initial value of the property
 * 3. A setter to assign the current property value
 */
template<typename T>
class SpriteAnim {
  public:
    SpriteAnim( // Provide access to the Anim<> object that holds the value we will animate
                const std::function<ci::Anim<T>&(Sprite&)>& getAnim,
                // Answer the current value of the property we will animate
                const std::function<T(Sprite&)>& getStartValue,
                // Assign the new property value
                const std::function<void(const T&, Sprite&)>& assignValue)
        : mGetAnim(getAnim)
        , mGetStartValue(getStartValue)
        , mAssignValue(assignValue)
    {
    }

    ci::Anim<T>&        getAnim(Sprite& s) const        { return mGetAnim(s); }
    const T             getStartValue(Sprite& s) const  { return mGetStartValue(s); }
    const std::function<void(const T&, Sprite&)>&
                        getAssignValue() const          { return mAssignValue; }

  private:
    const std::function<ci::Anim<T>&(Sprite&)>          mGetAnim;
    const std::function<T(Sprite&)>                     mGetStartValue;
    const std::function<void(const T&, Sprite&)>        mAssignValue;
};

/**
 * \class ds::ui::SpriteAnimatable
 * Provide conveniences for the common properties that can
 * be animated on a sprite.
 */
class SpriteAnimatable {
  public:
    SpriteAnimatable();
    virtual ~SpriteAnimatable();

    static const SpriteAnim<float>&       ANIM_OPACITY();
    static const SpriteAnim<ci::Vec3f>&   ANIM_POSITION();
    static const SpriteAnim<ci::Vec3f>&   ANIM_SCALE();
    static const SpriteAnim<ci::Vec3f>&   ANIM_SIZE();

    void                                  animStop();

  public:
      ci::Anim<float>                     mAnimOpacity;
      ci::Anim<ci::Vec3f>                 mAnimPosition;
      ci::Anim<ci::Vec3f>                 mAnimScale;
      ci::Anim<ci::Vec3f>                 mAnimSize;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_SPRITEANIM_H_