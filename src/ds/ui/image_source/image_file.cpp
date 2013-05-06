#include "image_file.h"

#include "ds/app/image_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE         = 0;
const char          RES_FN_ATT        = 20;
const char          RES_FLAGS_ATT     = 21;

/**
 * \class FileGenerator
 * This does all the work of loading the file and transporting settings across the network.
 */
class FileGenerator : public ImageGenerator
{
public:
	FileGenerator(SpriteEngine& e)
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()) { }
	FileGenerator(SpriteEngine& e, const std::string& fn, const int f)
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()), mFilename(fn), mFlags(f) { }

	const ci::gl::Texture*		getImage() {
		if (mTexture && mTexture.getWidth() > 0 && mTexture.getHeight() > 0) return &mTexture;

		if (mToken.canAcquire()) {
			mToken.acquire(mFilename, mFlags);
		} else {
			float						fade;
			mTexture = mToken.getImage(fade);
		}
		return nullptr;
	}

	virtual void							writeTo(DataBuffer& buf) const {
		buf.add(RES_FN_ATT);
		buf.add(mFilename);

		buf.add(RES_FLAGS_ATT);
		buf.add(mFlags);
	}

	virtual bool							readFrom(DataBuffer& buf) {
		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FN_ATT) return false;
		mFilename = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FLAGS_ATT) return false;
		mFlags = buf.read<int>();

		return true;
	}

private:
	ImageToken								mToken;
	std::string								mFilename;
	int												mFlags;
	ci::gl::Texture						mTexture;
};

}

/**
 * \class ds::ui::ImageFile
 */
void ImageFile::install(ds::ImageRegistry& registry)
{
  BLOB_TYPE = registry.addGenerator([](ds::ui::SpriteEngine& se)->ImageGenerator* { return new FileGenerator(se); });
}

ImageFile::ImageFile(const std::string& filename, const int flags)
	: mFilename(filename)
	, mFlags(flags)
{
}

ImageGenerator* ImageFile::newGenerator(SpriteEngine& e) const
{
	return new FileGenerator(e, mFilename, mFlags);
}

} // namespace ui
} // namespace ds
