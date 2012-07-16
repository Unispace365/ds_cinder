#include "ds/ui/sprite/sprite_registry.h"

#include <assert.h>
#include "ds/debug/debug_defines.h"
#include <iostream>

namespace ds {
namespace ui {

SpriteRegistry::SpriteRegistry()
{
}

void SpriteRegistry::add(const char type, const std::function<Sprite*(SpriteEngine&)>& f)
{
  if (!mRegistered.empty() && mRegistered.find(type) != mRegistered.end()) {
    DS_DBG_CODE(std::cout << "ERROR SpriteRegistry::add() type (" << type << ") already exists" << std::endl);
    assert(false);
    return;
  }
  mRegistered[type] = f;
}

Sprite* SpriteRegistry::generate(const char type, SpriteEngine& se) const
{
  if (mRegistered.empty()) return nullptr;

  auto it = mRegistered.find(type);
  if (it == mRegistered.end() || !(it->second)) return nullptr;
  return it->second(se);
}

} // namespace ui
} // namespace ds