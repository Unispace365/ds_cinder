#include "image_source_file.h"

#include "ds/app/image_source_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE         = 0;
const char          RES_FN_ATT        = 20;
const char          RES_FLAGS_ATT     = 21;
}

/**
 * \class ds::ui::ImageSourceFile
 */
void ImageSourceFile::install(ds::ImageSourceRegistry& registry)
{
  BLOB_TYPE = registry.add([](ds::ui::SpriteEngine& se)->ImageSource* { return new ImageSourceFile(se); });
}

ImageSourceFile::ImageSourceFile(SpriteEngine& se, const std::string& filename, const int flags)
	: ImageSource(BLOB_TYPE)
	, mImageToken(se.getLoadImageService())
	, mFilename(filename)
	, mFlags(flags)
{
}

ImageSourceFile::ImageSourceFile(SpriteEngine& se)
	: ImageSource(BLOB_TYPE)
	, mImageToken(se.getLoadImageService())
{
}

const ci::gl::Texture* ImageSourceFile::getImage()
{
	if (mTexture && mTexture.getWidth() > 0 && mTexture.getHeight() > 0) return &mTexture;

	if (mImageToken.canAcquire()) {
		mImageToken.acquire(mFilename, mFlags);
	} else {
		float						fade;
		mTexture = mImageToken.getImage(fade);
	}
	return nullptr;
}

void ImageSourceFile::writeTo(DataBuffer& buf) const
{
	buf.add(RES_FN_ATT);
	buf.add(mFilename);

	buf.add(RES_FLAGS_ATT);
	buf.add(mFlags);
}

bool ImageSourceFile::readFrom(DataBuffer& buf)
{
  if (!buf.canRead<char>()) return false;
  if (!buf.read<char>() == RES_FN_ATT) return false;
	mFilename = buf.read<std::string>();

  if (!buf.canRead<char>()) return false;
  if (!buf.read<char>() == RES_FLAGS_ATT) return false;
  mFlags = buf.read<int>();

	return true;
}

} // namespace ui
} // namespace ds
