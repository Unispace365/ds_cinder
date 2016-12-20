#include "image_arc.h"

#include <cinder/ImageIo.h>
#include <cinder/Surface.h>
#include "ds/app/environment.h"
#include "ds/app/image_registry.h"
#include "ds/arc/arc_io.h"
#include "ds/arc/arc_render_circle.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/logger.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE		= 0;
const char			W_ATT			= 20;
const char			H_ATT			= 21;
const char			FN_ATT			= 22;
const char			INPUT_ATT		= 23;
const char			WRITEFN_ATT		= 24;

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
	ArcGenerator(SpriteEngine&, const int width, const int height, const std::string& fn, const ds::arc::Input& input, const std::string& write_file)
		: ImageGenerator(BLOB_TYPE), mStatus(STATUS_EMPTY), mWidth(width), mHeight(height), mFilename(fn), mInput(input), mWriteFile(write_file) { }

	bool						getMetaData(ImageMetaData& d) const {
		d.mSize.x = static_cast<float>(mWidth);
		d.mSize.y = static_cast<float>(mHeight);
		return !d.empty();
	}

	const ci::gl::TextureRef		getImage() {
		if (mStatus == STATUS_EMPTY) generate();
		if (mStatus == STATUS_OK) return mTextureRef;
		return nullptr;
	}

	virtual void							writeTo(DataBuffer& buf) const {
		buf.add(W_ATT);
		buf.add(mWidth);

		buf.add(H_ATT);
		buf.add(mHeight);

		buf.add(FN_ATT);
		buf.add(mFilename);

		buf.add(INPUT_ATT);
		mInput.writeTo(buf);

		// Don't allow this to replicate, it's just a debugging thing.
//		buf.add(WRITEFN_ATT);
//		buf.add(mWriteFile);
	}

	virtual bool							readFrom(DataBuffer& buf) {
		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != W_ATT) return false;
		mWidth = buf.read<int>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != H_ATT) return false;
		mHeight = buf.read<int>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != FN_ATT) return false;
		mFilename = buf.read<std::string>();

		if (!buf.canRead<char>()) return false;
		if (buf.read<char>() != INPUT_ATT) return false;
		mInput.readFrom(buf);

		return true;
	}

	virtual std::string						getImageFilename() const {
		return mFilename;
	}

private:
	void											generate() {
		mStatus = STATUS_ERROR;
		if (mWidth < 1 || mHeight < 1) return;
		std::unique_ptr<ds::arc::Arc>		a = std::move(ds::arc::load(mFilename));
		if (!a) return;
		ci::Surface8u		s(mWidth, mHeight, true, ci::SurfaceConstraintsDefault());
		if (!s.getData() || s.getWidth() != mWidth || s.getHeight() != mHeight) return;

		ds::arc::RenderCircle		render;
		if (!render.on(mInput, s, *(a.get()))) return;

		writeFile(s);
		mTextureRef = ci::gl::Texture::create(s);
		if(mTextureRef && mTextureRef->getWidth() == mWidth && mTextureRef->getHeight() == mHeight) {
			mStatus = STATUS_OK;
		}
	}

	void					writeFile(const ci::Surface8u& s) {
		if (s.getWidth() > 0 && s.getHeight() > 0 && !mWriteFile.empty()) {
			try {
				const std::string		fn = ds::Environment::expand(mWriteFile);
				ci::writeImage(fn, s);
			} catch (std::exception const&) {
				DS_LOG_WARNING("Error writing rendered arc to file (" << mWriteFile << ")");
			}
		}
	}

	int						mStatus;
	int						mWidth,
							mHeight;
	std::string				mFilename;
	ds::arc::Input			mInput;
	std::string				mWriteFile;
	ci::gl::TextureRef		mTextureRef;
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
		, mFilename(filename) {
}

ImageGenerator* ImageArc::newGenerator(SpriteEngine& e) const {
	return new ArcGenerator(e, mWidth, mHeight, mFilename, mInput, mWriteFile);
}

void ImageArc::addColorInput(const ci::ColorA& c) {
	mInput.addColor(c);
}

void ImageArc::addFloatInput(const double f) {
	mInput.addFloat(f);
}

void ImageArc::addVec2Input(const ci::dvec2& v) {
	mInput.addVec2(v);
}

void ImageArc::setWriteFile(const std::string& fn) {
	mWriteFile = fn;
}

} // namespace ui
} // namespace ds

