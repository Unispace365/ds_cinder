#include "image_resource.h"

#include "ds/app/image_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE         = 0;
const char          RES_RES_ATT        = 20;
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
	FileGenerator(SpriteEngine& e, const ds::Resource& res, const int f)
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()), mResource(res), mFlags(f) { }

	const ci::gl::Texture*		getImage() {
		if (mTexture && mTexture.getWidth() > 0 && mTexture.getHeight() > 0) return &mTexture;

		if (mToken.canAcquire()) {
			mToken.acquire(mResource.getAbsoluteFilePath(), mFlags);
		} else {
			float						fade;
			mTexture = mToken.getImage(fade);
		}
		return nullptr;
	}

	virtual void							writeTo(DataBuffer& buf) const {
		buf.add(RES_RES_ATT);
		buf.add(mResource.getAbsoluteFilePath());

		buf.add(RES_FLAGS_ATT);
		buf.add(mFlags);
	}

	virtual bool							readFrom(DataBuffer& buf) {
		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_RES_ATT) return false;
		mResource = ds::Resource(buf.read<std::string>(), ds::Resource::IMAGE_TYPE);

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FLAGS_ATT) return false;
		mFlags = buf.read<int>();

		return true;
	}

private:
	ImageToken				mToken;
	ds::Resource			mResource;
	int						mFlags;
	ci::gl::Texture			mTexture;
};

}

/**
 * \class ds::ui::ImageResource
 */
void ImageResource::install(ds::ImageRegistry& registry) {
	BLOB_TYPE = registry.addGenerator([](ds::ui::SpriteEngine& se)->ImageGenerator* { return new FileGenerator(se); });
}

ImageResource::ImageResource(const ds::Resource& res, const int flags)
		: mResource(res)
		, mFlags(flags) {
}

ImageGenerator* ImageResource::newGenerator(SpriteEngine& e) const {
	return new FileGenerator(e, mResource, mFlags);
}

} // namespace ui
} // namespace ds
