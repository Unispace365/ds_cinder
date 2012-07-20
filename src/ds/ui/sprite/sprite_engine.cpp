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

std::unique_ptr<ci::gl::Fbo> SpriteEngine::getFbo( int width, int height )
{
  DS_VALIDATE(width > 0 && height > 0, return nullptr);

  if (!mFbos.empty()) {
    auto it = mFbos.begin();
    for (auto it2 = mFbos.end(); it != it2; ++it) {
      if ((*it).get()->getWidth() >= width && (*it).get()->getHeight() >= height) {
        std::unique_ptr<ci::gl::Fbo> fbo = std::move(*it);
        mFbos.erase(it);
        return std::move(fbo);
      }
    }

    std::unique_ptr<ci::gl::Fbo> fbo = std::move(mFbos.front());
    mFbos.pop_front();

    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGBA32F);
    format.setTarget(GL_TEXTURE_2D);
    fbo = std::move(std::unique_ptr<ci::gl::Fbo>(new ci::gl::Fbo(width, height, format)));

    return std::move(fbo);
  }

  gl::Fbo::Format format;
  format.setColorInternalFormat(GL_RGBA32F);
  format.setTarget(GL_TEXTURE_2D);
  return std::move(std::unique_ptr<ci::gl::Fbo>(new ci::gl::Fbo(width, height, format)));
}

void SpriteEngine::giveBackFbo( std::unique_ptr<ci::gl::Fbo> &fbo )
{
  mFbos.push_back(std::move(fbo));
}

}
}