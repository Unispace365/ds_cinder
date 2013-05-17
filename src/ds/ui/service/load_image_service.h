#pragma once
#ifndef DS_UI_SERVICE_LOADIMAGESERVICE_H_
#define DS_UI_SERVICE_LOADIMAGESERVICE_H_

#include <vector>
#include <unordered_map>
#include <cinder/Surface.h>
#include <cinder/gl/Texture.h>
#include "ds/thread/gl_thread.h"

namespace ds {

namespace ui {
class LoadImageService;

/* DS::IMAGE-TOKEN
 ******************************************************************/
class ImageToken {
public:
	ImageToken(LoadImageService&);
	~ImageToken();

	bool					canAcquire() const;
	/**
	 * \brief Request the resource be loaded.
	 * \param resourceId is the resource to load.
	 * \param filename is the filename (and path) for the resource ID.
	 * \param flags provides scope info (i.e. ds::IMG_CACHE).
	 */
	void					acquire(const std::string& filename, const int flags);
	void					release();

	ci::gl::Texture			getImage(float& fade);
	// No refs are acquired, no image is loaded -- if it exists, answer it
	const ci::gl::Texture	peekImage(const std::string& filename) const;

private:
	LoadImageService&		mSrv;
	std::string				mFilename;
	bool					mAcquired;
	bool					mError;
	ci::gl::Texture			mTexture;

	void					init();
};

/* DS::LOAD-IMAGE-SERVICE
 ******************************************************************/
class LoadImageService : public ds::GlThreadClient<LoadImageService> {
public:
	LoadImageService(GlThread&);
	~LoadImageService();

	// Clients should call release() for every successful acquire
	bool						acquire(const std::string& filename, const int flags);
	void						release(const std::string& filename);

	ci::gl::Texture				getImage(const std::string& filename, float& fade);
	// No refs are acquired, no image is loaded -- if it exists, answer it
	const ci::gl::Texture		peekImage(const std::string& filename) const;
	// Answer true if the token exists (though the image might not be loaded), supplying the flags if you like
	bool						peekToken(const std::string& filename, int* flags = nullptr) const;

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
		op(const std::string& filename, const int flags);

		void					clear();

		std::string				mFilename;
// This seems to cause problems with garbled images
//		ci::gl::Texture			mTexture;
		ci::Surface8u			mSurface;
		int						mFlags;
	};

private:
	void						_load();

	std::unordered_map<std::string, holder>
								mImageResource;

	Poco::Mutex					mMutex;
	// Input and output stacks for thread processing
	std::vector<op>				mInput, mOutput, mTmp;
};

} // namespace ui

} // namespace ds

#endif // DS_UI_SERVICE_LOADIMAGESERVICE_H_