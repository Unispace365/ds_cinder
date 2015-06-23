#pragma once
#ifndef DS_UI_SERVICE_LOADIMAGESERVICE_H_
#define DS_UI_SERVICE_LOADIMAGESERVICE_H_

#include <unordered_map>
#include <vector>
#include <cinder/Surface.h>
#include <cinder/gl/Texture.h>
#include "ds/app/engine/engine_service.h"
#include "ds/thread/gl_thread.h"
#include "ds/ui/ip/ip_function_list.h"

namespace ds {
namespace ui {
class LoadImageService;

/**
 * \class ds::ui::ImageKey
 * \brief Internal class that acts as a key for a cached image.
 */
class ImageKey {
public:
	ImageKey();
	ImageKey(const std::string& filename, const std::string& ip_key, const std::string& ip_params, const int flags);

	bool					operator==(const ImageKey&) const;
	void					clear();

	std::string				mFilename,
							mIpKey,
							mIpParams;
	int						mFlags;
};

} // namespace ui
} // namespace ds

/* \cond Ignore this function in Doxygen
	Make the ImageKey available for hashing functions
*/
namespace std {
	template<>
	struct hash<ds::ui::ImageKey> : public unary_function<ds::ui::ImageKey, size_t> {
		size_t operator()(const ds::ui::ImageKey& id) const {
			std::size_t h1 = std::hash<std::string>()(id.mFilename);
			std::size_t h2 = std::hash<std::string>()(id.mIpKey);
			return h1 ^ (h2 << 1);
		}
	};
}
/* \endcond */

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageToken
 * \brief Essentially a reference on an image.
 */
class ImageToken {
public:
	ImageToken(LoadImageService&);
	~ImageToken();

	// Answer true if I have neither an actual image nor a request for one.
	bool					empty() const;
	bool					canAcquire() const;
	/**
	 * \brief Request the resource be loaded.
	 * \param resourceId is the resource to load.
	 * \param filename is the filename (and path) for the resource ID.
	 * \param ip_key is a key to an IpFunction, which must be one of the
	 * system ones in ip_defs.h, or installed by the app.
	 * \param ip_params is parameters to the IpFunction. Format is dependent
	 * on the function.
	 * \param flags provides scope info (i.e. ds::IMG_CACHE).
	 */
	void					acquire(const std::string& filename, const std::string& ip_key,
									const std::string& ip_params, const int flags);
	void					release();

	ci::gl::Texture			getImage(float& fade);
	// No refs are acquired, no image is loaded -- if it exists, answer it
	const ci::gl::Texture	peekImage(const std::string& filename) const;

private:
	void					init();

	LoadImageService&		mSrv;
	ImageKey				mKey;
//	std::string				mFilename;
	bool					mAcquired;
	bool					mError;
	ci::gl::Texture			mTexture;
};

/**
 * \class ds::ui::LoadImageService
 * \brief Manage and load images.
 */
class LoadImageService : public ds::GlThreadClient<LoadImageService> {
public:
	LoadImageService(GlThread&, ds::ui::ip::FunctionList&);
	~LoadImageService();

	// Clients should call release() for every successful acquire
	bool						acquire(const ImageKey& key, const int flags);
	void						release(const ImageKey& key);

	ci::gl::Texture				getImage(const ImageKey&, float& fade);
	// No refs are acquired, no image is loaded -- if it exists, answer it
	const ci::gl::Texture		peekImage(const ImageKey&) const;
	// Answer true if the token exists (though the image might not be loaded), supplying the flags if you like
	bool						peekToken(const ImageKey&, int* flags = nullptr) const;

	void						update();
	void						clear();

private:
	// store a single image slot
	struct holder {
		holder();

		int						mRefs;
		ci::gl::Texture			mTexture;
		bool					mError;
		int						mFlags;
	};

	// an op for loading images
	struct op {
		op();
		op(const op&);
		op(const ImageKey&, const int flags, const ds::ui::ip::FunctionRef&);

		void					clear();

		ImageKey				mKey;
// This seems to cause problems with garbled images
//		ci::gl::Texture			mTexture;
		ci::Surface8u			mSurface;
		int						mFlags;
		ds::ui::ip::FunctionRef	mIpFunction;
	};

private:
	void						_load();

	ds::ui::ip::FunctionList&	mFunctions;
	// Hmm, had problems getting the hashing implemented for ImageKey
//	std::unordered_map<ImageKey, holder>
	std::unordered_map<ImageKey, holder>
								mImageResource;

	Poco::Mutex					mMutex;
	// Input and output stacks for thread processing
	std::vector<op>				mInput, mOutput, mTmp;
};

} // namespace ui
} // namespace ds

#endif