#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/resource_list.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/file_name_parser.h"

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE         = 0;

const DirtyState&   RES_ID_DIRTY 		  = INTERNAL_A_DIRTY;
const DirtyState&   RES_FN_DIRTY 		  = INTERNAL_B_DIRTY;

const char          RES_ID_ATT        = 80;
const char          RES_FN_ATT        = 81;
}

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
  setUseShaderTextuer(true);

  try {
    Vec2f size = parseFileMetaDataSize(filename);
    mWidth = size.x;
    mHeight = size.y;
  } catch (ParseFileMetaException &e) {
    std::cout << e.what() << std::endl;
    std::cout << "Going to load image synchronously; this will affect performance." << std::endl;
  }
  setTransparent(false);
  markAsDirty(RES_FN_DIRTY);
}

Image::Image( SpriteEngine& engine, const ds::Resource::Id &resourceId )
  : inherited(engine)
  , mImageService(engine.getLoadImageService())
  , mImageToken(mImageService)
  , mFlags(0)
  , mResourceId(resourceId)
{
  mBlobType = BLOB_TYPE;
  setUseShaderTextuer(true);

  ds::Resource            res;
  if (engine.getResources().get(resourceId, res)) {
    inherited::setSize(res.getWidth(), res.getHeight());
    mResourceFn = res.getAbsoluteFilePath();
  }
  setTransparent(false);
  markAsDirty(RES_ID_DIRTY);
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

void Image::writeAttributesTo(ds::DataBuffer& buf)
{
  inherited::writeAttributesTo(buf);

	if (mDirty.has(RES_ID_DIRTY)) {
    buf.add(RES_ID_ATT);
    mResourceId.writeTo(buf);
  }
	if (mDirty.has(RES_FN_DIRTY)) {
    buf.add(RES_FN_ATT);
    buf.add(mResourceFn);
  }
}

void Image::readAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
    if (attributeId == RES_ID_ATT) {
      mResourceId.readFrom(buf);
    } else if (attributeId == RES_FN_ATT) {
      mResourceFn = buf.read<std::string>();
    } else {
      inherited::readAttributeFrom(attributeId, buf);
    }
}

} // namespace ui
} // namespace ds
