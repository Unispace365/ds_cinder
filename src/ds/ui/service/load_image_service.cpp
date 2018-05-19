#include "stdafx.h"

#include "load_image_service.h"

#include <chrono>

#include <ds/util/file_meta_data.h>
#include <ds/debug/logger.h>


namespace ds {
namespace ui {

LoadImageService::LoadImageService(ds::ui::SpriteEngine& eng)
	: ds::AutoUpdate(eng, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
	, mShouldQuit(false)
{
}

void LoadImageService::initialize() {
	int numThreads = mEngine.getEngineSettings().getInt("load_image:threads");
	if(numThreads != mThreads.size()) {
		mShouldQuit = true;
		for(auto it : mThreads) {
			it->join();
		}
		mThreads.clear();
	}

	//clearCache();

	mShouldQuit = false;
	if(mThreads.empty()) {
		for(int i = 0; i < numThreads; i++) {
			ci::gl::ContextRef backgroundCtx = ci::gl::Context::create(ci::gl::context());
			auto aThread = std::shared_ptr<std::thread>(new std::thread(std::bind(&LoadImageService::loadImagesThreadFn, this, backgroundCtx)));
			mThreads.emplace_back(aThread);
		}
	}
}

void LoadImageService::clearCache() {
	mInUseImages.clear();
	ImageMetaData::clearMetadataCache();
}

void LoadImageService::logCache() {
	DS_LOG_INFO("Load Image Service, number of in use images:" << mInUseImages.size());
	for (auto it : mInUseImages){
		DS_LOG_INFO("Image, refs=" << it.second.mRefs << " err=" << it.second.mError << " flags=" << it.second.mFlags << " path=" << it.second.mFilePath);
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
			if(!it.second.mLoading && !it.second.mTexture && !it.second.mError) {
				mRequests[it.second.mFilePath] = it.second;
			}
		}
	}

	// cache or track completed loads
	for(auto& it : newCompletedRequests) {
		if(it.mTexture && it.mTexture->getId() > 0){
			auto findy = mInUseImages.find(it.mFilePath);
			if(findy == mInUseImages.end()) {
				mInUseImages[it.mFilePath] = it;
			} else {
				// this shouldn't really ever happen, but just in case
				DS_LOG_INFO("LoadImageService: somehow we ended up with duplicate loads for image " << it.mFilePath);
				findy->second.mRefs += it.mRefs;
			}
		} else {
			continue;
		}
		DS_LOG_VERBOSE(5, "LoadImageService completed loading " << it.mTexture << " error=" << it.mError << " refs=" << it.mRefs);

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
		DS_LOG_VERBOSE(6, "LoadImageService got a blank file path.");
		return;
	}

	if(!loadedCallback) {
		DS_LOG_WARNING("LoadImageService We need a callback for after the thing has been loaded");
		return;
	}

	// See if this has already been loaded
	auto& inFind = mInUseImages.find(filePath);
	if(inFind != mInUseImages.end()) {
		inFind->second.mRefs++;
		DS_LOG_VERBOSE(4, "LoadImageService using an in-use image for " << filePath << " refs=" << inFind->second.mRefs);
		loadedCallback(inFind->second.mTexture, inFind->second.mError, inFind->second.mErrorMsg);
		return;
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
		auto requestFindy = mRequests.find(filePath);
		if(requestFindy != mRequests.end()) {
			existsAlready = true;
			requestFindy->second.mRefs++;
			if((flags&Image::IMG_CACHE_F) && (requestFindy->second.mFlags&Image::IMG_CACHE_F) == 0) requestFindy->second.mFlags |= Image::IMG_CACHE_F;
			if((flags&Image::IMG_ENABLE_MIPMAP_F) && (requestFindy->second.mFlags&Image::IMG_ENABLE_MIPMAP_F) == 0) requestFindy->second.mFlags |= Image::IMG_ENABLE_MIPMAP_F;
		}


		if(!existsAlready) {
			mRequests[filePath] = (ilr);
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

	auto inFind = mInUseImages.find(filePath);
	if(inFind != mInUseImages.end()) {
		inFind->second.mRefs--;
		if(inFind->second.mRefs < 1 && (inFind->second.mFlags&Image::IMG_CACHE_F) == 0) {
			mInUseImages.erase(filePath);
			DS_LOG_VERBOSE(4, "LoadImageService no more refs for " << filePath);
		}
	}
}

void LoadImageService::loadImagesThreadFn(ci::gl::ContextRef context) {
	DS_LOG_VERBOSE(1, "Starting load thread " << std::this_thread::get_id());
	ci::ThreadSetup threadSetup;

	/// Make the shared context current
	context->makeCurrent();

	while(!mShouldQuit) {
		ImageLoadRequest nextImage;

		{
			std::lock_guard<std::mutex> lock(mMutex);
			for(auto& it : mRequests) {
				if(!it.second.mLoading) {
					it.second.mLoading = true;
					it.second.mTexture = nullptr;
					nextImage = it.second;
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

			// TODO: re-implement url loading
		//	if(ds::safeFileExistsCheck(nextImage.mFilePath)) {
				isr = ci::loadImage(nextImage.mFilePath);
		//	} else {
		//		isr = ci::loadImage(ci::loadUrl(nextImage.mFilePath));
		//	}

			ci::gl::Texture::Format	fmt;
			if((nextImage.mFlags&ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0) {
				fmt.enableMipmapping(true);
				fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
			}

			auto tex = ci::gl::Texture::create(isr, fmt);


			if(tex->getId() > 0) {
				{
					// we need to wait on a fence before alerting the primary thread that the Texture is ready
					auto fence = ci::gl::Sync::create();
					fence->clientWaitSync(1U, 1000000);
					nextImage.mTexture = tex;
				}
			
				{
					std::lock_guard<std::mutex> lock(mMutex);
					mLoadedRequests.emplace_back(nextImage);
				}
			} else {
				DS_LOG_VERBOSE(6, "Invalid texture, retrying for image " << nextImage.mFilePath << " " << std::this_thread::get_id());
				{
					std::lock_guard<std::mutex> lock(mMutex);
					auto findy = mRequests.find(nextImage.mFilePath);
					if(findy != mRequests.end()) {
						findy->second.mLoading = false;
					}
					
				}
				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
		} // end of try / catch
	} // end of while loop

	DS_LOG_VERBOSE(1, "Exiting load thread " << std::this_thread::get_id());
}

}
}