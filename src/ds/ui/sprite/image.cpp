#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/ui/sprite/sprite_engine.h"

namespace {

std::map<std::string, std::shared_ptr<ci::gl::Texture>> mTextureCache;

}

namespace ds {
namespace ui {

Image::Image( SpriteEngine& engine, const std::string &filename )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
    , mResourceFn(filename)
{
    setTransparent(false);
}

Image::~Image()
{
}

void Image::drawLocal()
{
  if (!mTexture) {
    // XXX Do bounds check here
    if (mImageToken.canAcquire()) { // && intersectsLocalScreen){
      requestImage();
    }
    float         fade;
    mTexture = mImageToken.getImage(fade);
    // Keep up the bounds
    if (mTexture) {
      Sprite::setSize(static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight()));
    }
    return;
  }

  ci::gl::draw(mTexture);
}

void Image::setSize( float width, float height )
{
    setScale( width / getWidth(), height / getHeight() );
}

void Image::loadImage( const std::string &filename )
{
  std::cout << "Image::loadImage() Still makin' this compatibile with async loading..." << std::endl;
#if 0
    mTexture = getImage( filename );
    float prevWidth = getWidth() * getScale().x;
    float prevHeight = getHeight() * getScale().y;

    if ( !mTexture )
        return;

    Sprite::setSize(static_cast<float>(mTexture->getWidth()), static_cast<float>(mTexture->getHeight()));
    setSize(prevWidth, prevHeight);
#endif
}

void Image::requestImage()
{
  // XXX Check to see if I have a resource ID, and use that instead.
  if (mResourceFn.empty()) return;

  mImageToken.acquire(mResourceFn, mFlags);
}

} // namespace ui
} // namespace ds
