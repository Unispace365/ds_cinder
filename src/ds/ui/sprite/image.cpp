#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/data/resource_list.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/sprite/sprite_registry.h"
#include "ds/util/file_name_parser.h"

namespace {
const char          SPRITE_TYPE       = 'i';
}

namespace ds {
namespace ui {

void Image::addTo(SpriteRegistry& registry)
{
  registry.add(SPRITE_TYPE, [](SpriteEngine& se)->Sprite*{return new Image(se);});
}

Image::Image( SpriteEngine& engine )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
{
  mSpriteType = SPRITE_TYPE;
  setTransparent(false);
}

Image::Image( SpriteEngine& engine, const std::string &filename )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
    , mResourceFn(filename)
{
  mSpriteType = SPRITE_TYPE;
  try {
    Vec2f size = parseFileMetaDataSize(filename);
    mWidth = size.x;
    mHeight = size.y;
  } catch (ParseFileMetaException &e) {
    std::cout << e.what() << std::endl;
    std::cout << "Going to load image synchronously; this will affect performance." << std::endl;
  }
  setTransparent(false);
}

Image::Image( SpriteEngine& engine, const ds::Resource::Id &resourceId )
  : inherited(engine)
  , mImageService(engine.getLoadImageService())
  , mImageToken(mImageService)
  , mFlags(0)
  , mResourceId(resourceId)
{
  mSpriteType = SPRITE_TYPE;
  setTransparent(false);
  ds::Resource            res;
  if (engine.getResources().get(resourceId, res)) {
    inherited::setSize(res.getWidth(), res.getHeight());
    mResourceFn = res.getAbsoluteFilePath();
  }
  setTransparent(false);
}

Image::~Image()
{
}

void Image::drawLocalClient()
{
  if (!inBounds())
    return;

  if (!mTexture) {
    // XXX Do bounds check here
    if (mImageToken.canAcquire()) { // && intersectsLocalScreen){
      requestImage();
    }
    float         fade;
    mTexture = mImageToken.getImage(fade);
    // Keep up the bounds
    if (mTexture) {
      const float         prevRealW = getWidth(), prevRealH = getHeight();
      if (prevRealW <= 0 || prevRealH <= 0) {
        Sprite::setSize(static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight()));
      } else {
        float             prevWidth = prevRealW * getScale().x;
        float             prevHeight = prevRealH * getScale().y;
        Sprite::setSize(static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight()));
        setSize(prevWidth, prevHeight);
      }
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
  mTexture.reset();
  mResourceFn = filename;
  requestImage();
}

void Image::requestImage()
{
  if (mResourceFn.empty()) return;

  mImageToken.acquire(mResourceFn, mFlags);
}

bool Image::isLoaded() const
{
  return mTexture;
}

} // namespace ui
} // namespace ds
