#pragma once
#ifndef DS_UI_SPRITE_SPRITEREGISTRY_H_
#define DS_UI_SPRITE_SPRITEREGISTRY_H_

#include <functional>
#include <unordered_map>

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;

/**
 * \class ds::ui::SpriteRegistry
 * Global sprite factory.
 */
class SpriteRegistry {
  public:
    SpriteRegistry();

    void              add(const char type, const std::function<Sprite*(SpriteEngine&)>&);
    Sprite*           generate(const char type, SpriteEngine&) const;

  private:
    std::unordered_map<char, std::function<Sprite*(SpriteEngine&)>>
                      mRegistered;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_SPRITEREGISTRY_H_