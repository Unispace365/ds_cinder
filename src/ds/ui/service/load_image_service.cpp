#include "stdafx.h"

#include "load_image_service.h"

#include <chrono>

#include <ds/debug/logger.h>
#include <ds/util/file_meta_data.h>

namespace {
template <typename Clock = std::chrono::high_resolution_clock>
class GenericStopWatch {
  public:
	typedef const typename Clock::time_point TimePoint;

	GenericStopWatch()
	  : mStartPoint(now()) {}

	static const TimePoint now() { return Clock::now(); }

	template <typename Rep = typename Clock::duration::rep, typename Units = typename Clock::duration>
	Rep elapsedTime() const {
		std::atomic_thread_fence(std::memory_order_relaxed);
		auto counted_time = std::chrono::duration_cast<Units>(Clock::now() - mStartPoint).count();
		std::atomic_thread_fence(std::memory_order_relaxed);
		return static_cast<Rep>(counted_time);
	}

	unsigned elapsedAttos() { return elapsedTime<unsigned, std::chrono::attoseconds>(); }
	unsigned elapsedFemtos() { return elapsedTime<unsigned, std::chrono::femtoseconds>(); }
	unsigned elapsedPicos() { return elapsedTime<unsigned, std::chrono::picoseconds>(); }
	unsigned elapsedNanos() { return elapsedTime<unsigned, std::chrono::nanoseconds>(); }
	unsigned elapsedMicros() { return elapsedTime<unsigned, std::chrono::microseconds>(); }
	unsigned elapsedMillis() { return elapsedTime<unsigned, std::chrono::milliseconds>(); }

  protected:
	TimePoint mStartPoint;
};

using StopWatch			 = GenericStopWatch<>;
using SystemStopWatch	 = GenericStopWatch<std::chrono::system_clock>;
using MonotonicStopWatch = GenericStopWatch<std::chrono::steady_clock>;

} // anonymous namespace


namespace ds { namespace ui {

	LoadImageService::LoadImageService(ds::ui::SpriteEngine& eng)
	  : ds::AutoUpdate(eng, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
	  , mShouldQuit(false)
	  , mTextureOnMainThread(false)
	  , mCacheEverything(false) {
		mEngine.getEngineSettings().getSetting("load_image:create_texture_on_main_thread", 0,
											   ds::cfg::SETTING_TYPE_BOOL,
											   "True will create the gl texture on the main thread. Only use if you "
											   "have issues with delayed image loading, typically in fullscreen",
											   "false");
		mTextureOnMainThread =
			mEngine.getEngineSettings().getBool("load_image:create_texture_on_main_thread", 0, false);

		mEngine.getEngineSettings().getSetting("load_image:cache_everything", 0, ds::cfg::SETTING_TYPE_BOOL,
											   "True will keep all images in GPU memory until the app exits. False "
											   "only caches the images loaded with the cache flag",
											   "false");
		mCacheEverything = mEngine.getEngineSettings().getBool("load_image:cache_everything", 0, false);
	}

	void LoadImageService::initialize() {
		const int numThreads = mEngine.getEngineSettings().getInt("load_image:threads");
		if (numThreads != mThreads.size()) {
			stopThreads();
		}

		mEngine.timedCallback(
			[this, numThreads] {
				while (mThreads.size() < (size_t)numThreads) {
					ci::gl::ContextRef backgroundCtx = ci::gl::Context::create(ci::gl::context());

					auto aThread =
						std::make_shared<std::thread>([this, backgroundCtx]() { loadImagesThreadFn(backgroundCtx); });
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
			DS_LOG_INFO("Image, refs=" << it.second.mRefs << " err=" << it.second.mError
									   << " flags=" << it.second.mFlags << " path=" << it.second.mFilePath);
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


	void LoadImageService::handleImageLoadRequest(ImageLoadRequest& request) {
		const bool doMipMapping = ((request.mFlags & ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0);

		ci::gl::Texture::Format fmt;
		if (doMipMapping) {
			fmt.enableMipmapping(true);
			fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		}

		auto tex = ci::gl::Texture::create(request.mImageSourceRef, fmt);
		if (!tex || tex->getId() < 1) {
			DS_LOG_WARNING("LoadImageService: couldn't load an image texture on the main thread for "
						   << request.mFilePath);
		}

		request.mTexture		= tex;
		request.mImageSourceRef = nullptr;
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
			auto findy = mInUseImages.find(it.mFilePath);
			if (findy == mInUseImages.end()) {
				DS_LOG_VERBOSE(3, "Image loaded after no one was left to care!" << it.mFilePath);
				// Don't cache this image if its no longer used
				continue;
			}

			// This is a fallback in case gl fencing for thread creation stalls
			if (mTextureOnMainThread) {
				handleImageLoadRequest(it);
			}

			if (it.mTexture && it.mTexture->getId() > 0) {
				DS_LOG_VERBOSE(5, "LoadImageService completed loading " << it.mTexture << " error=" << it.mError
																		<< " refs=" << it.mRefs);
			} else {
				DS_LOG_WARNING("LoadImageService failed for file: " << it.mFilePath);
			}

			mInUseImages[it.mFilePath] = it;

			// Run the callbacks for the requested image path
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
			if (inFind->second.mLoading == false) {
				DS_LOG_VERBOSE(4, "LoadImageService using an in-use image for " << filePath
																				<< " refs=" << inFind->second.mRefs);
				loadedCallback(inFind->second.mTexture, inFind->second.mError, inFind->second.mErrorMsg);
				return;
			}
		} else {
			mInUseImages[filePath]			= ImageLoadRequest(filePath, flags);
			mInUseImages[filePath].mLoading = true; // Indicates that this request has been added to the loading queue
		}

		// Add callback
		mCallbacks[filePath][requester] = loadedCallback;

		// ok, this image isn't cached and it's not currently in use/loading, start a new load request
		{
			std::lock_guard<std::mutex> lock(mRequestsMutex);

			// Skip if there's already a request for the same image file path
			const auto it = std::find_if(mRequests.begin(), mRequests.end(), [&filePath](const auto& e) {
				return e.mLoading == true && e.mFilePath == filePath;
			});
			if (it == mRequests.end()) mRequests.push_back(mInUseImages[filePath]);
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

		if (mCacheEverything) return;

		bool wasRemoved = false;
		auto inFind		= mInUseImages.find(filePath);
		if (inFind != mInUseImages.end()) {
			inFind->second.mRefs--;
			if (inFind->second.mRefs < 1 && (inFind->second.mFlags & Image::IMG_CACHE_F) == 0) {
				mInUseImages.erase(filePath);
				wasRemoved = true;
				DS_LOG_VERBOSE(4, "LoadImageService no more refs for " << filePath);
			}
		}

		// Also remove this from the pending mRequests queue so the background thread doesn't try to load it...
		if (wasRemoved) {
			std::lock_guard<std::mutex> lock(mRequestsMutex);
			const auto					requestFind = std::find_if(mRequests.begin(), mRequests.end(),
																   [&filePath](const auto& e) { return e.mFilePath == filePath; });
			if (requestFind != mRequests.end()) {
				mRequests.erase(requestFind);
				DS_LOG_VERBOSE(4, "LoadImageService: Removing request for: " << filePath);
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

		// Using a PBO to upload textures seems to reduce stuttering...
		const bool usePbo = true;

		// Create a PBO to load image data into
		const int pboW		  = 4096;
		const int pboH		  = 4096;
		const int pboChannels = 4;
		const int pboSize	  = pboW * pboH * pboChannels;

		GLubyte* dummyBuf = new GLubyte[pboSize];
		for (int i = 0; i < pboSize; i++) {
			dummyBuf[i] = (GLubyte)(i);
		}
		auto cinderPbo = ci::gl::Pbo::create(GL_PIXEL_UNPACK_BUFFER, pboSize, dummyBuf, GL_STATIC_DRAW);
		delete dummyBuf;

		while (!mShouldQuit) {
			ImageLoadRequest nextImage;
			bool			 gotRequest = false;

			{
				std::lock_guard<std::mutex> lock(mRequestsMutex);
				if (!mRequests.empty()) {
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

			// Setup texture format
			const bool doMipMapping = ((nextImage.mFlags & ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0);

			ci::gl::Texture2d::Format fmt;
			if (doMipMapping) {
				fmt.enableMipmapping(true);
				fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
			} else {
				fmt.setMinFilter(GL_LINEAR);
			}
			// fmt.loadTopDown(false);

			try {
				ci::ImageSourceRef isr;
				try {
					isr = ci::loadImage(nextImage.mFilePath);
				} catch (std::exception excp) {
					isr = ci::loadImage(ci::loadUrl(nextImage.mFilePath));
				}

				/// once we have the surface in main-thread mode, bail out
				if (mTextureOnMainThread && isr) {
					nextImage.mImageSourceRef = isr;
					nextImage.mLoading		  = false;
					std::lock_guard<std::mutex> lock(mLoadedMutex);
					mLoadedRequests.emplace_back(nextImage);
					continue;
				}

				// DS_LOG_INFO("Creating texture for loaded image: " << nextImage.mFilePath);

				ci::gl::TextureRef tex;
				const int		   w = isr->getWidth();
				const int		   h = isr->getHeight();

				if (usePbo) {
					// NH: If we don't set the texture internal format or type, then Cinder will automatically infer the
					// format from the image This allows us to e.g. load HDR EXR images in float32 or float16 formats...
					// const GLint internalFormat = isr->hasAlpha() ? GL_RGBA : GL_RGB;
					// fmt.setInternalFormat(internalFormat);
					// fmt.dataType(GL_UNSIGNED_BYTE);
					fmt.setIntermediatePbo(cinderPbo);
				}

				tex = ci::gl::Texture::create(isr, fmt);

				if (tex->getId() > 0) {

					std::this_thread::sleep_for(30ms);

					{
						// we need to wait on a fence before alerting the primary thread that the Texture is ready
						auto fence = ci::gl::Sync::create();

						// NH: waitSync() waits for the OpenGL SERVER, and does not guarantee that the signal is
						// triggered because it triggered, and not because of the timeout
						// ... waitClientSync waits for OpenGL CLIENT, and DOES guarantee that the signal was triggered,
						// or timed out.

						// one milisecond
						const auto timeoutNanos = 1'000'000ull;
						StopWatch  syncTimer;
						int		   numWaits = 1;
						bool	   success	= false;
						while (true) {
							const auto syncReturn = fence->clientWaitSync(GL_SYNC_FLUSH_COMMANDS_BIT, timeoutNanos);
							const auto elapsed	  = syncTimer.elapsedMicros();
							if (syncReturn == GL_CONDITION_SATISFIED) {
								DS_LOG_VERBOSE(2, "LoadImageService::Sync success after "
													  << numWaits << " tries, and " << elapsed << " microseconds");
								success = true;
								break;
							} else if (syncReturn == GL_ALREADY_SIGNALED) {
								DS_LOG_VERBOSE(2, "LoadImageService::Sync success after "
													  << numWaits << " tries, and " << elapsed
													  << " microseconds, already signaled!");
								success = true;
								break;
							} else if (syncReturn == GL_WAIT_FAILED) {
								DS_LOG_WARNING("LoadImageService::Sync failure after "
											   << numWaits << " tries, and " << elapsed << " microseconds...");
								break;
							} else if (syncReturn == GL_TIMEOUT_EXPIRED) {
								numWaits++;
								continue;
							} else {
								DS_LOG_WARNING("LoadImageService::Unknown sync failure!");
								break;
							}
						}

						if (success) {
							nextImage.mTexture = tex;
							nextImage.mLoading = false;
						} else {
							const auto elapsed = syncTimer.elapsedMicros();
							DS_LOG_WARNING("Failed to sync texture after " << numWaits << " tries, and " << elapsed
																		   << " microseconds...");
							nextImage.mError	= true;
							nextImage.mErrorMsg = "Could not sync Texture";
						}

						std::lock_guard<std::mutex> lock(mLoadedMutex);
						mLoadedRequests.emplace_back(nextImage);
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
				if (exc.what()) {
					DS_LOG_WARNING("Failed to create texture for image " << nextImage.mFilePath
																		 << " what: " << exc.what());
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
			} // end of try / catch
		}	  // end of while loop

		// DS_LOG_VERBOSE(1, "Exiting load thread " << std::this_thread::get_id());
	}

}} // namespace ds::ui
