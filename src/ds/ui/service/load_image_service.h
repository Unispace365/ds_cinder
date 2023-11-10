#pragma once
#ifndef DS_UI_SERVICE_LOAD_IMAGE_SERVICE
#define DS_UI_SERVICE_LOAD_IMAGE_SERVICE

#include <cinder/Thread.h>
#include <cinder/gl/Texture.h>
#include <ds/app/auto_update.h>

namespace ds::ui {
class Image;
class SpriteEngine;

/**
 * \class LoadImageService
 * \brief Loads images into textures on multiple threads.
 */
class LoadImageService : public ds::AutoUpdate {
  public:
	typedef std::function<void(ci::gl::TextureRef, ci::Rectf, const bool errored, const std::string& errMsg)> LoadedCallback;

	LoadImageService(SpriteEngine& eng);
	~LoadImageService();

	/// TESTS:
	///  - Cached retains image indefinitely
	///  - Image texture goes away if there were no caches for it
	///  - No memory leaks

	/// \brief Asynchronously get an image at the specified path or url
	/// Flags are for caching (IMG_CACHE_F) or mipmapping (IMG_MIPMAP_F)
	/// Each requester can only get 1 callback.
	/// Important! Be sure to call release before the requester goes away
	/// The callback will be called one time only, and calls back if there is an error or it succeeds.
	/// All callbacks happen in the update cycle
	void acquire(const std::string& filePath, const int flags, Image* requester, const LoadedCallback& loadedCallback);

	/// You must call release if you no longer want the image or the reffer is about to be released
	void release(const std::string& filePath, Image* requester);

	/// \brief Starts the threads to load stuff and creates OpenGL contexts
	/// Can be called multiple times, will reinit the loading threads if the load_image:threads
	/// setting has been changed.
	void initialize();

	/// \brief Clears references to all loaded images
	/// Wont' clear images currently held by sprites
	/// But will force all new images to load from scratch
	/// This also clears the metadata cache
	void clearCache();

	/// Logs all in-use and cached images to info
	void logCache();

  private:
	/// Keeps track of requests for images, in-use images, and cached images
	struct ImageLoadRequest {
		ImageLoadRequest() = default;

		ImageLoadRequest(const std::string& filePath, int flags, const ci::Rectf& coords)
		  : mFilePath(filePath)
		  , mFlags(flags)
		  , mTexCoords(coords)
		  , mRefs(1) {}

		std::string		   mFilePath;
		int				   mFlags = 0;
		bool			   mError = false;
		std::string		   mErrorMsg;
		ci::Rectf		   mTexCoords{0, 0, 1, 1}; // only used for cropping and trimming white space - TODO rename?
		ci::gl::TextureRef mTexture;
		ci::ImageSourceRef mImageSourceRef; // only for main-thread image creation
		int				   mRefs	= 0;
		bool			   mLoading = false;
	};


	std::unordered_map<std::string, std::unordered_map<void*, LoadedCallback>> mCallbacks;

	virtual void update(const ds::UpdateParams&) override;
	/// Stop all running threads and clear shared_ptr's
	void stopThreads();

	/// If the cache flag is present, store a reference to the texture
	std::unordered_map<std::string, ImageLoadRequest> mInUseImages;

	void loadImagesThreadFn(ci::gl::ContextRef context);

	std::vector<std::shared_ptr<std::thread>> mThreads;
	/// shared between threads

	mutable std::mutex			  mRequestsMutex;
	std::vector<ImageLoadRequest> mRequests;

	mutable std::mutex			  mLoadedMutex;
	std::vector<ImageLoadRequest> mLoadedRequests;

	bool mShouldQuit;

	bool mTextureOnMainThread;
	bool mCacheEverything;

  protected:
	void handleImageLoadRequest(ImageLoadRequest&);
};

} // namespace ds::ui
#endif
