#include "image_file.h"

#include "ds/app/image_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE			= 0;
const char          RES_FN_ATT			= 20;
const char          RES_FLAGS_ATT		= 21;
const char          RES_IPKEY_ATT		= 22;
const char          RES_IPPARAMS_ATT	= 23;

/**
 * \class FileGenerator
 * This does all the work of loading the file and transporting settings across the network.
 */
class FileGenerator : public ImageGenerator {
public:
	FileGenerator(SpriteEngine& e)
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()) { }
	FileGenerator(SpriteEngine& e, const std::string& fn, const std::string& ip_key, const std::string& ip_params, const int f)
			: ImageGenerator(BLOB_TYPE), mToken(e.getLoadImageService()), mFilename(fn), mIpKey(ip_key), mIpParams(ip_params), mFlags(f) { preload(); }

	const std::string&			getFilename() const {
		return mFilename;
	}

	virtual std::string			getImageFilename() const {
		return mFilename;
	}

	const int					getFlags() const {
		return mFlags;
	}

	bool						getMetaData(ImageMetaData& d) const {
		if (mFilename.empty()) return false;
		ImageMetaData			atts(mFilename);
		d = atts;
		return !d.empty();
	}

	const ci::gl::Texture*		getImage() {
		if (mTexture) return &mTexture;

		if (mToken.canAcquire()) {
			mToken.acquire(mFilename, mIpKey, mIpParams, mFlags);
		}
		float						fade;
		mTexture = mToken.getImage(fade);
		if (mTexture) return &mTexture;
		return nullptr;
	}

	virtual void				writeTo(DataBuffer& buf) const {
		buf.add(RES_FN_ATT);
		buf.add(mFilename);

		buf.add(RES_IPKEY_ATT);
		buf.add(mIpKey);

		buf.add(RES_IPPARAMS_ATT);
		buf.add(mIpParams);

		buf.add(RES_FLAGS_ATT);
		buf.add(mFlags);
	}

	virtual bool				readFrom(DataBuffer& buf) {
		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FN_ATT) return false;
		mFilename = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_IPKEY_ATT) return false;
		mIpKey = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_IPPARAMS_ATT) return false;
		mIpParams = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FLAGS_ATT) return false;
		mFlags = buf.read<int>();

		preload();

		return true;
	}

private:
	void					preload() {
		// XXX This should check to see if I'm in client mode and only
		// load it then. (or the service should be empty in server mode).
		if ((mFlags&ds::ui::Image::IMG_PRELOAD_F) != 0 && mToken.canAcquire()) {
			mToken.acquire(mFilename, mIpKey, mIpParams, mFlags);
		}
	}

	ImageToken				mToken;
	std::string				mFilename;
	std::string				mIpKey,
							mIpParams;
	int						mFlags;
	ci::gl::Texture			mTexture;
};

}

/**
 * \class ds::ui::ImageFile
 */
void ImageFile::install(ds::ImageRegistry& registry) {
	BLOB_TYPE = registry.addGenerator([](ds::ui::SpriteEngine& se)->ImageGenerator* { return new FileGenerator(se); });
}

ImageFile::ImageFile(const std::string& filename, const int flags)
		: mFilename(filename)
		, mFlags(flags) {
}

ImageFile::ImageFile(	const std::string& filename, const std::string& ip_key,
						const std::string& ip_params, const int flags)
		: mFilename(filename)
		, mIpKey(ip_key)
		, mIpParams(ip_params)
		, mFlags(flags) {
}

ImageGenerator* ImageFile::newGenerator(SpriteEngine& e) const {
	return new FileGenerator(e, mFilename, mIpKey, mIpParams, mFlags);
}

bool ImageFile::generatorMatches(const ImageGenerator& gen) const {
	const FileGenerator*	fgen = dynamic_cast<const FileGenerator*>(&gen);
	if (fgen) {
		return mFilename == fgen->getFilename() && mFlags == fgen->getFlags();
	}
	return false;
}

} // namespace ui
} // namespace ds
