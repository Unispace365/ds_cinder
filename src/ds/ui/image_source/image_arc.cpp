#include "image_arc.h"

#include <cinder/Surface.h>
#include "ds/app/image_registry.h"
#include "ds/arc/arc_io.h"
#include "ds/arc/arc_render_circle.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE		= 0;
const char			RES_FN_ATT		= 20;
const char			RES_FLAGS_ATT	= 21;

const int			STATUS_ERROR	= -1;
const int			STATUS_EMPTY	= 0;
const int			STATUS_OK		= 1;

/**
 * \class ArcGenerator
 * This does all the work of generating and transporting settings across the network.
 */
class ArcGenerator : public ImageGenerator
{
public:
	ArcGenerator(SpriteEngine&)
		: ImageGenerator(BLOB_TYPE), mStatus(STATUS_EMPTY), mWidth(0), mHeight(0) { }
	ArcGenerator(SpriteEngine&, const int width, const int height, const std::string& fn, const ds::arc::Input& input)
		: ImageGenerator(BLOB_TYPE), mStatus(STATUS_EMPTY), mWidth(width), mHeight(height), mFilename(fn), mInput(input) { }

	bool						getMetaData(ImageMetaData& d) const {
		d.mSize.x = static_cast<float>(mWidth);
		d.mSize.y = static_cast<float>(mHeight);
		return !d.empty();
	}

	const ci::gl::Texture*		getImage() {
		if (mStatus == STATUS_EMPTY) generate();
		if (mStatus == STATUS_OK) return &mTexture;
		return nullptr;
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
		mStatus = STATUS_ERROR;
		if (mWidth < 1 || mHeight < 1) return;
		std::unique_ptr<ds::arc::Arc>		a = std::move(ds::arc::load(mFilename));
		if (!a) return;
		ci::Surface8u		s(mWidth, mHeight, true, ci::SurfaceConstraintsDefault());
		if (!s || s.getWidth() != mWidth || s.getHeight() != mHeight) return;

		ds::arc::RenderCircle		render;
		if (!render.on(mInput, s, *(a.get()))) return;

		mTexture = ci::gl::Texture(s);
		if (mTexture && mTexture.getWidth() == mWidth && mTexture.getHeight() == mHeight) {
			mStatus = STATUS_OK;
		}
	}

	int						mStatus;
	const int				mWidth,
							mHeight;
	std::string				mFilename;
	ds::arc::Input			mInput;
	ci::gl::Texture			mTexture;
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
	return new ArcGenerator(e, mWidth, mHeight, mFilename, mInput);
}

void ImageArc::addColorInput(const ci::ColorA& c)
{
	mInput.addColor(c);
}

void ImageArc::addFloatInput(const double f)
{
	mInput.addFloat(f);
}

void ImageArc::addVec2Input(const ci::Vec2d& v)
{
	mInput.addVec2(v);
}

} // namespace ui
} // namespace ds
