#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/resource_list.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/file_name_parser.h"

namespace {
char          BLOB_TYPE       = 0;
}

namespace ds {
namespace ui {

void Image::installAsServer(ds::BlobRegistry& registry)
{
  BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Image::installAsClient(ds::BlobRegistry& registry)
{
  BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Image>(r);});
}

Image::Image( SpriteEngine& engine )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
{
  mBlobType = BLOB_TYPE;
  setTransparent(false);
}

Image::Image( SpriteEngine& engine, const std::string &filename )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
    , mResourceFn(filename)
{
  mBlobType = BLOB_TYPE;
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
  mBlobType = BLOB_TYPE;
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
