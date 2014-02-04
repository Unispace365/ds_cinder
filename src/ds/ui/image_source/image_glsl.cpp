#include "image_glsl.h"

#include <cinder/Surface.h>
#include <ds/app/environment.h>
#include "ds/app/image_registry.h"
#include "ds/arc/arc_io.h"
#include "ds/arc/arc_render_circle.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/service/glsl_image_service.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE		= 0;
const char			RES_FN_ATT		= 20;
const char			RES_FLAGS_ATT	= 21;

/**
 * \class Generator
 * This does all the work of generating and transporting settings across the network.
 */
class Generator : public ImageGenerator
{
public:
	Generator(SpriteEngine& e)
			: ImageGenerator(BLOB_TYPE), mToken(e.getService<ds::glsl::ImageService>(ds::glsl::IMAGE_SERVICE)), mWidth(0), mHeight(0) { }
	Generator(SpriteEngine& e, const int width, const int height, const std::string& vfn, const std::string& ffn, const ds::gl::Uniform& uniform)
			: ImageGenerator(BLOB_TYPE), mToken(e.getService<ds::glsl::ImageService>(ds::glsl::IMAGE_SERVICE)), mWidth(width), mHeight(height), mVertexFilename(vfn), mFragmentFilename(ffn), mUniform(uniform) {
		ds::glsl::ImageKey		key;
		key.setTo(vfn, ffn, uniform, width, height, 0);
		mToken.setTo(key);
	}

	bool						getMetaData(ImageMetaData& d) const {
		d.mSize.x = static_cast<float>(mWidth);
		d.mSize.y = static_cast<float>(mHeight);
		return !d.empty();
	}

	const ci::gl::Texture*		getImage() {
		if (!mTexture) {
			float				fade(1.0f);
			mTexture = mToken.getImage(fade);
		}
		if (mTexture && mTexture.getWidth() > 0 && mTexture.getHeight() > 0) return &mTexture;
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
	ds::glsl::ImageToken	mToken;
	const int				mWidth,
							mHeight;
	std::string				mVertexFilename,
							mFragmentFilename;
	ds::gl::Uniform			mUniform;
	ci::gl::Texture			mTexture;

};

}

/**
 * \class ds::ui::ImageGlsl
 */
void ImageGlsl::install(ds::ImageRegistry& registry) {
	BLOB_TYPE = registry.addGenerator([](ds::ui::SpriteEngine& se)->ImageGenerator* { return new Generator(se); });
}

namespace {
std::string		shader_file(const std::string& filestem, const std::string& extension) {
	return ds::Environment::getAppFile("data/shaders/" + filestem + "." + extension);	
}
}

ImageGlsl::ImageGlsl(const int width, const int height, const std::string& filestem)
	: mWidth(width)
	, mHeight(height)
	, mVertexFilename(shader_file(filestem, "vert"))
	, mFragmentFilename(shader_file(filestem, "frag")) {
}

ImageGlsl::ImageGlsl(const int width, const int height, const std::string& vertexFilename, const std::string& fragmentFilename)
	: mWidth(width)
	, mHeight(height)
	, mVertexFilename(vertexFilename)
	, mFragmentFilename(fragmentFilename) {
}

ImageGenerator* ImageGlsl::newGenerator(SpriteEngine& e) const {
	return new Generator(e, mWidth, mHeight, mVertexFilename, mFragmentFilename, mUniform);
}

ds::gl::Uniform& ImageGlsl::getUniform() {
	return mUniform;
}

} // namespace ui
} // namespace ds
