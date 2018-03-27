#include "stdafx.h"

#include "load_image_service.h"

#include <chrono>

#include <ds/util/file_meta_data.h>
#include <ds/debug/logger.h>


namespace ds {
namespace ui {

LoadImageService::LoadImageService(ds::ui::SpriteEngine& eng)
	: ds::AutoUpdate(eng)
	, mShouldQuit(false)
{
}

void LoadImageService::initialize() {
	if(mThreads.empty()) {
		for(int i = 0; i < 4; i++) {
			ci::gl::ContextRef backgroundCtx = ci::gl::Context::create(ci::gl::context());
			auto aThread = std::shared_ptr<std::thread>(new std::thread(std::bind(&LoadImageService::loadImagesThreadFn, this, backgroundCtx)));
			mThreads.emplace_back(aThread);
		}
	}
}

LoadImageService::~LoadImageService() {
	mCallbacks.clear();

	mShouldQuit = true;

	for(auto it : mThreads) {
		it->join();
	}
}

void LoadImageService::update(const ds::UpdateParams&) {

	// grab any completed image loads and clear the shared vector
	std::vector<ImageLoadRequest> newCompletedRequests;
	{
		std::lock_guard<std::mutex> lock(mMutex);
		newCompletedRequests = mLoadedRequests;
		mLoadedRequests.clear();

		auto oldRequests = mRequests;
		mRequests.clear();
		for (auto it : oldRequests){
			if(!it.mLoaded) mRequests.emplace_back(it);
		}
	}

	// cache or track completed loads
	for(auto& it : newCompletedRequests) {
		if(it.mTexture && it.mFlags&Image::IMG_CACHE_F) {
			mCachedImages.emplace_back(it);
		} else {
			it.mRefs++;
			mInUseImages.emplace_back(it);
		}
		DS_LOG_VERBOSE(1, "LoadImageService completed loading " << it.mTexture << " error=" << it.mError << " refs=" << it.mRefs);

		auto filecallbacks = mCallbacks.find(it.mFilePath);
		if(filecallbacks != mCallbacks.end()) {
			for (auto cit : filecallbacks->second){
				cit.second(it.mTexture, it.mError, it.mErrorMsg);
			}

			mCallbacks.erase(filecallbacks);
		}
	}

	newCompletedRequests.clear();

}

void LoadImageService::acquire(const std::string& filePath, const int flags, void * requester, LoadedCallback loadedCallback) {
	if(filePath.empty()) {
		DS_LOG_WARNING("LoadImageService got a blank file path.");
		return;
	}

	if(!loadedCallback) {
		DS_LOG_WARNING("LoadImageService We need a callback for after the thing has been loaded");
		return;
	}

	// Check if this was cached already (doesn't matter if the new flags have cached or not, since this will always exist)
	for(auto it : mCachedImages) {
		if(it.mFilePath == filePath && !it.mError) {
			DS_LOG_VERBOSE(1, "LoadImageService using a cached image for " << filePath);
			loadedCallback(it.mTexture, it.mError, it.mErrorMsg);
			return;
		}
	}

	// See if this has already been loaded
	for(auto& it : mInUseImages) {
		if(it.mFilePath == filePath && !it.mError) {
			it.mRefs++;
			DS_LOG_VERBOSE(1, "LoadImageService using a non-cached in-use image for " << filePath << " refs=" << it.mRefs);
			loadedCallback(it.mTexture, it.mError, it.mErrorMsg);
			return;
		}
	}

	auto findy = mCallbacks.find(filePath);
	if(findy != mCallbacks.end()) {
		findy->second[requester] = loadedCallback;
	} else {
		mCallbacks[filePath][requester] = loadedCallback;
	}

	// ok, this image isn't cached and it's not currently in use, start a new load request
	ImageLoadRequest ilr(filePath, flags);

	{
		std::lock_guard<std::mutex> lock(mMutex);

		bool existsAlready = false;
		for(auto& it : mRequests) {
			if(it.mFilePath == filePath) {
				existsAlready = true;
				it.mRefs++;
				if((flags&Image::IMG_CACHE_F) && (it.mFlags&Image::IMG_CACHE_F) == 0) it.mFlags |= Image::IMG_CACHE_F;
				if((flags&Image::IMG_ENABLE_MIPMAP_F) && (it.mFlags&Image::IMG_ENABLE_MIPMAP_F) == 0) it.mFlags |= Image::IMG_ENABLE_MIPMAP_F;
				break;
			}
		}

		if(!existsAlready) {
			mRequests.emplace_back(ilr);
		}
	}
}


void LoadImageService::release(const std::string& filePath, void * referrer) {
	if(filePath.empty()) return;

	/// Remove the callback for this path and referrer
	auto findy = mCallbacks.find(filePath);
	if(findy != mCallbacks.end()) {
		auto refFindy = findy->second.find(referrer);
		if(refFindy != findy->second.end()) {
			findy->second.erase(refFindy);
		}
	}

	for(auto it = mInUseImages.begin(); it < mInUseImages.end(); ++it) {
		if((*it).mFilePath == filePath) {
			(*it).mRefs--;
			if((*it).mRefs < 1) {
				mInUseImages.erase(it);
				DS_LOG_VERBOSE(1, "LoadImageService  no more refs for " << filePath);
			}
			break;
		}
	}
}

void LoadImageService::loadImagesThreadFn(ci::gl::ContextRef context) {
	ci::ThreadSetup threadSetup;

	/// Make the shared context current
	context->makeCurrent();

	while(!mShouldQuit) {
		ImageLoadRequest nextImage;

		{
			std::lock_guard<std::mutex> lock(mMutex);
			for(auto& it : mRequests) {
				if(!it.mLoaded) {
					it.mLoaded = true;
					nextImage = it;
					break;
				}
			}
			
		}

		// there was no filepaths, so wait and try again in a little bit (the time is a guess)
		if(nextImage.mFilePath.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		try {
			ci::ImageSourceRef isr = nullptr;

			if(ds::safeFileExistsCheck(nextImage.mFilePath)) {
				isr = ci::loadImage(nextImage.mFilePath);
			} else {
				isr = ci::loadImage(ci::loadUrl(nextImage.mFilePath));
			}

			ci::gl::Texture::Format	fmt;
			if((nextImage.mFlags&ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0) {
				fmt.enableMipmapping(true);
				fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
			}
			auto tex = ci::gl::Texture::create(isr, fmt);

			// we need to wait on a fence before alerting the primary thread that the Texture is ready
			auto fence = ci::gl::Sync::create();
			fence->clientWaitSync();

			nextImage.mTexture = tex;

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mLoadedRequests.emplace_back(nextImage);
			}

		} catch(std::exception &exc) {
			nextImage.mError = true;
			if(false && exc.what()) {
				DS_LOG_WARNING("Failed to create texture for image " << nextImage.mFilePath << " what: " << exc.what());
				nextImage.mErrorMsg = exc.what();
			} else {
				DS_LOG_WARNING("Failed to create texture for image " << nextImage.mFilePath);
				nextImage.mErrorMsg = "Unknown load issue.";
			}


			/// Send the error back out
			{
				std::lock_guard<std::mutex> lock(mMutex);
				mLoadedRequests.emplace_back(nextImage);
			}
		}
	}
}

}
}