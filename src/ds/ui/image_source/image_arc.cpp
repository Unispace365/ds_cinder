#include "image_arc.h"

#include "ds/app/image_registry.h"
#include "ds/arc/arc_io.h"
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

const int						STATUS_ERROR			= -1;
const int						STATUS_EMPTY			= 0;
const int						STATUS_OK					= 1;

/**
 * \class ArcGenerator
 * This does all the work of generating and transporting settings across the network.
 */
class ArcGenerator : public ImageGenerator
{
public:
	ArcGenerator(SpriteEngine&)
			: ImageGenerator(BLOB_TYPE), mStatus(STATUS_EMPTY) { }
	ArcGenerator(SpriteEngine&, const std::string& fn, const std::vector<float>& floatInput)
			: ImageGenerator(BLOB_TYPE), mStatus(STATUS_EMPTY), mFilename(fn), mFloatInput(floatInput) { }

	const ci::gl::Texture*		getImage() {
		if (mStatus == STATUS_EMPTY) generate();
		if (mStatus == STATUS_OK) return &mTexture;
		return nullptr;
#if 0
		if (mTexture && mTexture.getWidth() > 0 && mTexture.getHeight() > 0) return &mTexture;

		if (mToken.canAcquire()) {
			mToken.acquire(mFilename, mFlags);
		} else {
			float						fade;
			mTexture = mToken.getImage(fade);
		}
#endif
	}

	virtual void							writeTo(DataBuffer& buf) const {
		assert(false);
#if 0
		buf.add(RES_FN_ATT);
		buf.add(mFilename);

		buf.add(RES_FLAGS_ATT);
		buf.add(mFlags);
#endif
	}

	virtual bool							readFrom(DataBuffer& buf) {
		assert(false);
#if 0
		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FN_ATT) return false;
		mFilename = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != RES_FLAGS_ATT) return false;
		mFlags = buf.read<int>();
#endif

		return true;
	}

private:
	void											generate() {
		std::unique_ptr<ds::arc::Arc>		a = std::move(ds::arc::load(mFilename));
		if (!a) {
			mStatus = STATUS_ERROR;
			return;
		}
		mStatus = STATUS_ERROR;
	}

	int												mStatus;
	std::string								mFilename;
	std::vector<float>				mFloatInput;
	ci::gl::Texture						mTexture;
};

}

/**
 * \class ds::ui::ImageFile
 */
void ImageArc::install(ds::ImageRegistry& registry)
{
  BLOB_TYPE = registry.addGenerator([](ds::ui::SpriteEngine& se)->ImageGenerator* { return new ArcGenerator(se); });
}

ImageArc::ImageArc(const int width, const int height, const std::string& filename)
	: mWidth(width)
	, mHeight(height)
	, mFilename(filename)
{
}

ImageGenerator* ImageArc::newGenerator(SpriteEngine& e) const
{
	return new ArcGenerator(e, mFilename, mFloatInput);
}

void ImageArc::addInput(const float f)
{
	mFloatInput.push_back(f);
}

} // namespace ui
} // namespace ds
