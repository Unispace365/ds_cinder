#include "sprite_engine.h"
#include "sprite.h"

namespace ds {
namespace ui {

void SpriteEngine::addToDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;
  
  removeFromDragDestinationList(sprite);

  mDragDestinationSprites.push_back(sprite);
}

void SpriteEngine::removeFromDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;

  auto found = std::find(mDragDestinationSprites.begin(), mDragDestinationSprites.end(), sprite);
  if (found != mDragDestinationSprites.end())
    mDragDestinationSprites.erase(found);
}

Sprite *SpriteEngine::getDragDestinationSprite( const ci::Vec3f &globalPoint, Sprite *draggingSprite )
{
  for (auto it = mDragDestinationSprites.begin(), it2 = mDragDestinationSprites.end(); it != it2; ++it) {
  	Sprite *sprite = *it;
    if (sprite == draggingSprite)
      continue;
    if (sprite->contains(globalPoint))
      return sprite;
  }

  return nullptr;
}

}
}