#include "stdafx.h"

#include "load_image_service.h"

#include <chrono>

#include <ds/debug/logger.h>
#include <ds/util/file_meta_data.h>


namespace ds {
namespace ui {

LoadImageService::LoadImageService(ds::ui::SpriteEngine& eng)
  : ds::AutoUpdate(eng, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
  , mShouldQuit(false) {}

void LoadImageService::initialize() {
	const int numThreads = mEngine.getEngineSettings().getInt("load_image:threads");
	if (numThreads != mThreads.size()) {
		stopThreads();
	}

	mEngine.timedCallback(
			[this, numThreads] {
				while (mThreads.size() < (size_t)numThreads) {
					ci::gl::ContextRef backgroundCtx = ci::gl::Context::create(ci::gl::context());

					auto aThread = std::make_shared<std::thread>(
							[this, backgroundCtx]() { loadImagesThreadFn(backgroundCtx); });
					mThreads.emplace_back(aThread);
				}
			},
			0.1);
}

void LoadImageService::clearCache() {
	mInUseImages.clear();
	ImageMetaData::clearMetadataCache();
}

void LoadImageService::logCache() {
	DS_LOG_INFO("Load Image Service, number of in use images:" << mInUseImages.size());
	for (auto it : mInUseImages) {
		DS_LOG_INFO("Image, refs=" << it.second.mRefs << " err=" << it.second.mError << " flags=" << it.second.mFlags
								   << " path=" << it.second.mFilePath);
	}
}

void LoadImageService::stopThreads() {
	mShouldQuit = true;

	for (auto it : mThreads) {
		it->join();
	}

	mThreads.clear();
	mShouldQuit = false;
}
LoadImageService::~LoadImageService() {
	mCallbacks.clear();

	stopThreads();
}

void LoadImageService::update(const ds::UpdateParams&) {
	// grab any completed image loads and clear the shared vector
	std::vector<ImageLoadRequest> newCompletedRequests;
	{
		std::lock_guard<std::mutex> lock(mLoadedMutex);
		newCompletedRequests = mLoadedRequests;
		mLoadedRequests.clear();
	}

	// cache or track completed loads
	for (auto& it : newCompletedRequests) {
		if (it.mTexture && it.mTexture->getId() > 0) {
			auto findy = mInUseImages.find(it.mFilePath);
			if (findy == mInUseImages.end()) {
				// mInUseImages[it.mFilePath] = it;
				DS_LOG_VERBOSE(3, "Image loaded after no one was left to care!" << it.mFilePath);
			} else {
				mInUseImages[it.mFilePath] = it;
				// this shouldn't really ever happen, but just in case
				// findy->second.mRefs += it.mRefs;
			}
		} else {
			DS_LOG_WARNING("LoadImgeService failed for file: " << it.mFilePath)
			continue;
		}
		DS_LOG_VERBOSE(5, "LoadImageService completed loading " << it.mTexture << " error=" << it.mError
																<< " refs=" << it.mRefs);

		auto filecallbacks = mCallbacks.find(it.mFilePath);
		if (filecallbacks != mCallbacks.end()) {
			for (auto cit : filecallbacks->second) {
				cit.second(it.mTexture, it.mError, it.mErrorMsg);
			}

			mCallbacks.erase(filecallbacks);
		}
	}

	newCompletedRequests.clear();
}

void LoadImageService::acquire(const std::string& filePath, const int flags, void* requester,
							   LoadedCallback loadedCallback) {
	if (filePath.empty()) {
		DS_LOG_VERBOSE(6, "LoadImageService got a blank file path.");
		return;
	}

	if (!loadedCallback) {
		DS_LOG_ERROR("LoadImageService We need a callback for after the thing has been loaded");
		return;
	}

    // Check if this has already been loaded or requested.

	// See if this has already been loaded
	auto& inFind = mInUseImages.find(filePath);
	if (inFind != mInUseImages.end()) {
        // Increment the ref counter
		inFind->second.mRefs++;

        // If this image has already loaded, fire the callback immediately 
        if(inFind->second.mLoading == false){
            DS_LOG_VERBOSE(4,
                           "LoadImageService using an in-use image for " << filePath << " refs=" << inFind->second.mRefs);
            loadedCallback(inFind->second.mTexture, inFind->second.mError, inFind->second.mErrorMsg);
            return;
        }
	}else{
        mInUseImages[filePath] = ImageLoadRequest(filePath, flags);
        mInUseImages[filePath].mLoading = true; // Indicates that this request has been added to the loading queue
    }

    // Add callback
	mCallbacks[filePath][requester] = loadedCallback;

	// ok, this image isn't cached and it's not currently in use/loading, start a new load request
	{
		std::lock_guard<std::mutex> lock(mRequestsMutex);
        mRequests.push_back(mInUseImages[filePath]);
	}
}


void LoadImageService::release(const std::string& filePath, void* referrer) {
	if (filePath.empty()) return;

	/// Remove the callback for this path and referrer
	auto findy = mCallbacks.find(filePath);
	if (findy != mCallbacks.end()) {
		auto refFindy = findy->second.find(referrer);
		if (refFindy != findy->second.end()) {
			findy->second.erase(refFindy);
		}
	}

	auto inFind = mInUseImages.find(filePath);
	if (inFind != mInUseImages.end()) {
		inFind->second.mRefs--;
		if (inFind->second.mRefs < 1 && (inFind->second.mFlags & Image::IMG_CACHE_F) == 0) {
			mInUseImages.erase(filePath);
			DS_LOG_VERBOSE(4, "LoadImageService no more refs for " << filePath);
		}
	}
}

void LoadImageService::loadImagesThreadFn(ci::gl::ContextRef context) {
    // Allow using 10ms vs std::chrono::milliseconds(10)
    using namespace std::chrono_literals;

	DS_LOG_VERBOSE(1, "Starting load thread " << std::this_thread::get_id());
	ci::ThreadSetup threadSetup;

	/// Make the shared context current
	context->makeCurrent();

	while (!mShouldQuit) {
		ImageLoadRequest nextImage;
        bool gotRequest = false;

		{
			std::lock_guard<std::mutex> lock(mRequestsMutex);
            if(!mRequests.empty()){
                // Get first request from the queue
                nextImage = mRequests.front();
                mRequests.erase(mRequests.begin());
                gotRequest = true;
            }
		}

		// there was no filepaths, so wait and try again in a little bit (the time is a guess)
		if (!gotRequest) {
			std::this_thread::sleep_for(4ms);
			continue;
		}

		try {
			// TODO: re-implement url loading
			//	if(ds::safeFileExistsCheck(nextImage.mFilePath)) {
			auto isr = ci::loadImage(nextImage.mFilePath);
			//	} else {
			//		isr = ci::loadImage(ci::loadUrl(nextImage.mFilePath));
			//	}

			ci::gl::Texture::Format fmt;
			if ((nextImage.mFlags & ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0) {
				fmt.enableMipmapping(true);
				fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
			}

			auto tex = ci::gl::Texture::create(isr, fmt);


			if (tex->getId() > 0) {
				{
					// we need to wait on a fence before alerting the primary thread that the Texture is ready
					auto   fence = ci::gl::Sync::create();
					GLenum syncReturn;
					do {
						syncReturn = fence->clientWaitSync(1U, std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count());  // 1ms to nanoseconds
					} while (syncReturn == GL_TIMEOUT_EXPIRED);

					if (syncReturn == GL_WAIT_FAILED) {
						DS_LOG_WARNING("LoadImageService fence wait didn't work! " << syncReturn);
					} else {
						nextImage.mTexture = tex;
                        nextImage.mLoading = false;

						std::lock_guard<std::mutex> lock(mLoadedMutex);
						mLoadedRequests.emplace_back(nextImage);
					}
				}
			} else {
				DS_LOG_VERBOSE(6, "Invalid texture, retrying for image " << nextImage.mFilePath << " "
																		 << std::this_thread::get_id());
				{
					std::lock_guard<std::mutex> lock(mRequestsMutex);
                    mRequests.push_back(nextImage);
				}
			}

		} catch (std::exception& exc) {
			nextImage.mError = true;
			if (false && exc.what()) {
				DS_LOG_WARNING("Failed to create texture for image " << nextImage.mFilePath << " what: " << exc.what());
				nextImage.mErrorMsg = exc.what();
			} else {
				DS_LOG_WARNING("Failed to create texture for image " << nextImage.mFilePath);
				nextImage.mErrorMsg = "Unknown load issue.";
			}


			/// Send the error back out
			{
				std::lock_guard<std::mutex> lock(mLoadedMutex);
				mLoadedRequests.emplace_back(nextImage);
			}
		}  // end of try / catch
	}	  // end of while loop

	DS_LOG_VERBOSE(1, "Exiting load thread " << std::this_thread::get_id());
}

}  // namespace ui
}  // namespace ds
