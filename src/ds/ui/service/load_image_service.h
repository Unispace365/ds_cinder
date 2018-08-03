#pragma once
#ifndef DS_UI_SERVICE_LOAD_IMAGE_SERVICE
#define DS_UI_SERVICE_LOAD_IMAGE_SERVICE

#include <ds/app/auto_update.h>
#include <cinder/Thread.h>
#include <cinder/gl/Texture.h>

namespace ds {
namespace ui {
class SpriteEngine;

/**
 * \class LoadImageService
 * \brief Loads images into textures on multiple threads.
 */
class LoadImageService : public ds::AutoUpdate {
public:

	typedef std::function<void(ci::gl::TextureRef, const bool errored, const std::string& errMsg)> LoadedCallback;

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
	void acquire(const std::string& filePath, const int flags, void * requester, LoadedCallback loadedCallback);

	/// You must call release if you no longer want the image or the reffer is about to be released
	void release(const std::string& filePath, void * requester);

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
		ImageLoadRequest()
			: mFilePath("")
			, mFlags(0)
			, mRefs(0)
			, mLoading(false)
			, mTexture(nullptr)
		{}

		ImageLoadRequest(const std::string filePath, const int flags)
			: mFilePath(filePath)
			, mFlags(flags)
			, mError(false)
			, mRefs(1)
			, mLoading(false)
			, mTexture(nullptr)
		{
		}
		std::string						mFilePath;
		int								mFlags;
		bool							mError;
		std::string						mErrorMsg;
		ci::gl::TextureRef				mTexture;
		int								mRefs;
		bool							mLoading;
	};


	
	std::unordered_map<std::string, std::unordered_map<void *, LoadedCallback>> mCallbacks;

	virtual void update(const ds::UpdateParams&) override;
    /// Stop all running threads and clear shared_ptr's
    void stopThreads();

	/// If the cache flag is present, store a reference to the texture
	std::unordered_map<std::string, ImageLoadRequest>	mInUseImages;

	void												loadImagesThreadFn(ci::gl::ContextRef context);
	std::vector<std::shared_ptr<std::thread>>			mThreads;
	/// shared between threads

	mutable std::mutex									mRequestsMutex;
	std::vector<ImageLoadRequest>	                    mRequests;

	mutable std::mutex									mLoadedMutex;
	std::vector<ImageLoadRequest>						mLoadedRequests;

	bool												mShouldQuit;
};

}
}
#endif
