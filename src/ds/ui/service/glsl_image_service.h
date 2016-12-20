#pragma once
#ifndef DS_UI_SERVICE_GLSLIMAGESERVICE_H_
#define DS_UI_SERVICE_GLSLIMAGESERVICE_H_

#include <vector>
#include <unordered_map>
#include <cinder/Surface.h>
#include <cinder/gl/Texture.h>
#include <Poco/Mutex.h>
#include "ds/app/engine/engine_service.h"
#include "ds/gl/uniform.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace glsl {
class ImageService;

extern const std::string&		IMAGE_SERVICE;

/* \class ds::glsl::ImageKey
 * \brief A unique key for a single rendered image.
 */
class ImageKey {
public:
	ImageKey();

	bool					operator==(const ImageKey&) const;

	bool					empty() const;

	const std::string&		getVertex() const		{ return mVertex; }
	const std::string&		getFragment() const		{ return mFragment; }
	int						getWidth() const		{ return mW; }
	int						getHeight() const		{ return mH; }
	const gl::Uniform&		getUnifom() const		{ return mUniform; }

	/**
	 * \brief Set to the supplied settings.  This will not generate an
	 * actual image.
	 * \param vertex, fragment The IDs of the vertex and fragment shader.
	 * \param shaderValues is the shader and its configuration that generates the image.
	 * \param w is the width of the generated image.
	 * \param h is the height of the generated image.
	 * \param flags provides scope info (i.e. ds::IMG_CACHE).
	 */
	void					setTo(	const std::string& vertex, const std::string& fragment,
								  const ds::gl::Uniform& shaderValues,
									const int w, const int h, const int flags);
	void					clear();

private:
	std::string				mVertex,
							mFragment;
	ds::gl::Uniform			mUniform;
	int						mW, mH;
	int						mFlags;
};

/* \class ds::glsl::ImageToken
 */
class ImageToken {
public:
	ImageToken(ImageService&);
	~ImageToken();

	/**
	 * \brief Set to the supplied settings.  This will not generate an
	 * actual image.
	 * \param key is the key that uniquely describes the rendered image.
	 */
	void					setTo(const ImageKey&);
	void					clear();

	/**
	 * \brief Generate the image, if necessary, and answer.
	 */
	ci::gl::TextureRef		getImage(float& fade);

private:
	void					init();
	void					release();

	ImageService&			mSrv;
	ImageKey				mKey;
	bool					mAcquired;
	bool					mError;
	ci::gl::TextureRef		mTextureRef;
};

/* \class ds::glsl::ImageService
 */
class ImageService : public ds::EngineService {
public:
	ImageService(ds::ui::SpriteEngine&);
	~ImageService();

	// Clients should call release() for every successful acquire
	bool					acquire(const ImageKey&);
	void					release(const ImageKey&);

	ci::gl::TextureRef		getImage(const ImageKey&, float& fade);

	void					update();
	void					clear();

 private:
	// store a single image slot
	struct holder {
		holder();
		holder(const ImageKey&);

		ImageKey			mKey;
		int					mRefs;
		ci::gl::TextureRef	mImgRef;
		bool				mError;
	};

	// an op for rendering images
	struct op {
		op();
		op(const ImageKey&);

		// input
		ImageKey			mKey;
		// output
		ci::gl::TextureRef	mImgRef;
	};

private:
	holder*					find(const ImageKey&, int* index = nullptr);

	// NOTE:  This is running in the main thread.  I think it has to.
	void					renderInput();
	void					renderInput(op&);

	ds::ui::SpriteEngine&	mEngine;
	std::vector<holder>		mCache;

	Poco::Mutex				mMutex;
	// Input and output stacks for thread processing
	std::vector<op>			mInput, mOutput, mTmp;
};

} // namespace glsl
} // namespace ds

#endif // DS_UI_SERVICE_GLSLIMAGESERVICE_H_