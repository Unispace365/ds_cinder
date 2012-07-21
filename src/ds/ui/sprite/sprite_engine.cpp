#include "sprite_engine.h"
#include "sprite.h"
#include "ds/debug/debug_defines.h"

using namespace ci;

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

std::unique_ptr<FboGeneral> SpriteEngine::getFbo()
{
  //DS_VALIDATE(width > 0 && height > 0, return nullptr);

  if (!mFbos.empty()) {
    std::unique_ptr<FboGeneral> fbo = std::move(mFbos.front());
    mFbos.pop_front();
    return std::move(fbo);
  }

  std::unique_ptr<FboGeneral> fbo = std::move(std::unique_ptr<FboGeneral>(new FboGeneral()));
  fbo->setup();
  return std::move(fbo);
}

void SpriteEngine::giveBackFbo( std::unique_ptr<FboGeneral> &fbo )
{
  mFbos.push_back(std::move(fbo));
}

} // namespace ui
} // namespace ds
