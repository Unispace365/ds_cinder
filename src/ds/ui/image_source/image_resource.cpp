#include "image_resource.h"

#include "ds/app/image_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/resource_list.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/sprite/image.h"
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
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()), mResource(res), mFlags(f) { preload(); }

	const ds::Resource&			getResource() const {
		return mResource;
	}

	const int					getFlags() const {
		return mFlags;
	}

	bool						getMetaData(ImageMetaData& d) const {
		const std::string&		fn = mResource.getAbsoluteFilePath();
		if (fn.empty()) return false;
		ImageMetaData			atts(fn);
		d = atts;
		return !d.empty();
	}

	const ci::gl::Texture*		getImage() {
		if (mTexture) return &mTexture;

		if (mToken.canAcquire()) {
			mToken.acquire(mResource.getAbsoluteFilePath(), "", "", mFlags);
		}
		float						fade;
		mTexture = mToken.getImage(fade);
		if (mTexture) return &mTexture;
		return nullptr;
	}

	virtual void							writeTo(DataBuffer& buf) const {
		buf.add(RES_RES_ATT);
		buf.add(mResource.getPortableFilePath());

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

		preload();

		return true;
	}

private:
	void								preload() {
		// XXX This should check to see if I'm in client mode and only
		// load it then. (or the service should be empty in server mode).
		if ((mFlags&ds::ui::Image::IMG_PRELOAD_F) != 0 && mToken.canAcquire()) {
			mToken.acquire(mResource.getAbsoluteFilePath(), "", "", mFlags);
		}
	}

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
	ImageMetaData md;
	md.add(res.getAbsoluteFilePath(), ci::Vec2f(res.getWidth(), res.getHeight()));

}

ImageResource::ImageResource(const ds::Resource::Id& id, const int flags)
		: mResourceId(id)
		, mFlags(flags) {
}

ImageGenerator* ImageResource::newGenerator(SpriteEngine& e) const {
	ds::Resource			r(mResource);
	if (r.empty() && !mResourceId.empty()) {
		e.getResources().get(mResourceId, r);
	}
	return new FileGenerator(e, r, mFlags);
}

bool ImageResource::generatorMatches(const ImageGenerator& gen) const {
	const FileGenerator*	fgen = dynamic_cast<const FileGenerator*>(&gen);
	if (fgen) {
		return mResource == fgen->getResource() && mFlags == fgen->getFlags();
	}
	return false;
}

} // namespace ui
} // namespace ds
