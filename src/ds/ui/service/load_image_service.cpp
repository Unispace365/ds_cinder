#include "ds/ui/service/load_image_service.h"

#include <cinder/ImageIo.h>
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"
#include "Poco/File.h"

namespace {
const ds::BitMask	LOAD_IMAGE_LOG_M = ds::Logger::newModule("load_image");
// A mask of all the image flags that impact the key.
const int			IMAGE_FLAGS_KEY_MASK(ds::ui::Image::IMG_CACHE_F);
}

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageKey
 */
ImageKey::ImageKey()
		: mFlags(0) {
}

ImageKey::ImageKey(const std::string& filename, const std::string& ip_key, const std::string& ip_params, const int flags)
		: mFilename(filename)
		, mIpKey(ip_key)
		, mIpParams(ip_params)
		, mFlags(flags&IMAGE_FLAGS_KEY_MASK) {
}

bool ImageKey::operator==(const ImageKey& o) const {
	if (this == &o) return true;
	return mFilename == o.mFilename && mIpKey == o.mIpKey && mIpParams == o.mIpParams && mFlags == o.mFlags;
}

void ImageKey::clear() {
	mFilename.clear();
	mIpKey.clear();
	mIpParams.clear();
	mFlags = 0;
}

/**
 * \class ds::ui::ImageKey
 */
ImageToken::ImageToken(LoadImageService& _srv)
		: mSrv(_srv) {
	init();
}

ImageToken::~ImageToken() {
	release();
}

bool ImageToken::empty() const {
	return !mAcquired;
}

bool ImageToken::canAcquire() const {
	return !mAcquired && !mError;
}

void ImageToken::acquire(	const std::string& _filename, const std::string& ip_key,
							const std::string& ip_params, const int flags) {
	if (mAcquired) return;

	if (_filename.empty()) {
		mError = true;
//		DS_LOG_WARNING_M("ImageToken: Unable to load image resource (no filename)", LOAD_IMAGE_LOG_M);
		return;
	}
	const ImageKey			key(_filename, ip_key, ip_params, flags);
	mAcquired = mSrv.acquire(key, flags);
	if (mAcquired) {
		mKey = key;
	}
}

void ImageToken::release() {
	if (mAcquired) mSrv.release(mKey);
	init();
}

ci::gl::Texture ImageToken::getImage(float& fade) {
	if (!mAcquired) return ci::gl::Texture();

	if (!mTexture) {
		return (mTexture = mSrv.getImage(mKey, fade));
	}

	fade = 1;
	return mTexture;
}

const ci::gl::Texture ImageToken::peekImage(const std::string& filename) const {
	return mSrv.peekImage(mKey);
}

void ImageToken::init() {
	mKey.clear();
	mAcquired = false;
	mError = false;
	mTexture.reset();
}

/* DS::LOAD-IMAGE-SERVICE
 ******************************************************************/
LoadImageService::LoadImageService(ds::ui::SpriteEngine& eng, ds::ui::ip::FunctionList& list)
		: mFunctions(list) 
		, mLoadThreads(eng, [](){return new ds::ui::LoadImageService::ImageLoadThread(); })
		, mMaxSimultaneousLoads(4)
		, mMaxLoadTries(128)
		, mLoadsInProgress(0)
{

	mLoadThreads.setReplyHandler([this](ds::ui::LoadImageService::ImageLoadThread& q){ 
		onLoadComplete(q); 
	});
}

LoadImageService::~LoadImageService(){
	clear();
}

bool LoadImageService::acquire(const ImageKey& key, const int flags) {
	ImageHolder&		h = mImageResource[key];

	// We have to test multiple conditions here -- if our refs fall below 1 AND we have no
	// current image, then we need to load one in.  But if the refs are > 0, then there's
	// either an image or one's being loaded.  And if there's an image but the refs are < 1,
	// then it's being cached.
	if((!h.mTexture) && h.mRefs < 1) {
		// There's no image, so push on an operation to start one
		auto oppy = ImageOperation(key, flags, mFunctions.find(key.mIpKey));
		mOperationsQueue.push_back(oppy);
		advanceQueue();
	}

	h.mRefs++;
	if((flags&Image::IMG_CACHE_F) != 0) h.mFlags |= Image::IMG_CACHE_F;
	if((flags&Image::IMG_ENABLE_MIPMAP_F) != 0) h.mFlags |= Image::IMG_ENABLE_MIPMAP_F;

	return true;
}

void LoadImageService::advanceQueue(){
	if(mLoadsInProgress >= mMaxSimultaneousLoads){
		return;
	}

	if(mOperationsQueue.empty()) return;

	auto oppy = mOperationsQueue.front();
	oppy.mNumberTries++;
	mOperationsQueue.erase(mOperationsQueue.begin());

	mLoadsInProgress++;
	mLoadThreads.start([this, oppy](ImageLoadThread& ilt){ ilt.mOutput = oppy; });
}

void LoadImageService::release(const ImageKey& key) {
	// Note:  As far as I can tell, find() always throws an error if the map is empty.
	// Further, I can't even seem to catch the error, so really not sure what's going on there.
	if (mImageResource.empty()) {
	//	DS_LOG_WARNING_M("LoadImageService::release() called on empty map", LOAD_IMAGE_LOG_M);
		return;
	}

	auto			it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		ImageHolder&		h = it->second;
		h.mRefs--;
		// If I'm caching this image, never release it
		if ((h.mFlags&Image::IMG_CACHE_F) == 0 && h.mRefs <= 0) {
			mImageResource.erase(key);
		}
	} else {
		DS_LOG_WARNING_M("LoadImageService::release() called on filename that doesn't exist (" << key.mFilename << ")", LOAD_IMAGE_LOG_M);
	}
}

ci::gl::Texture LoadImageService::getImage(const ImageKey& key, float& fade) {

	if (mImageResource.empty()) return ci::gl::Texture();
	ImageHolder& h = mImageResource[key];
	fade = 1;
	return h.mTexture;
}

const ci::gl::Texture LoadImageService::peekImage(const ImageKey& key) const {
	if (mImageResource.empty()) return ci::gl::Texture();
	auto it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		const ImageHolder&		h = it->second;
		return h.mTexture;
	}
	return ci::gl::Texture();
}

bool LoadImageService::peekToken(const ImageKey& key, int* flags) const {
	if (mImageResource.empty()) return false;
	auto it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		const ImageHolder&		h = it->second;
		if (flags) *flags = h.mFlags;
		return true;
	}
	return false;
}

void LoadImageService::onLoadComplete(ImageLoadThread& loadThread){

	mLoadsInProgress--;

	// if something went wrong (out of memory? no file? try again)
	if(loadThread.mError){
		if(loadThread.mOutput.mNumberTries >= mMaxLoadTries){
			DS_LOG_WARNING("Gave up loading image for " << loadThread.mOutput.mKey.mFilename << " after " << loadThread.mOutput.mNumberTries << " attempts.");
			loadThread.mOutput.clear();
		} else {
			mOperationsQueue.push_back(loadThread.mOutput);
		}
		advanceQueue();
		return;
	}

	ImageOperation&			out = loadThread.mOutput;
	ImageHolder&			h = mImageResource[out.mKey];
	if(h.mTexture) {
		// This isn't an error any more, and is just fine. Really the problem is that we spent a bunch of time loading the same image twice
		//DS_LOG_WARNING_M("Duplicate images for id=" << out.mKey.mFilename << " refs=" << h.mRefs, LOAD_IMAGE_LOG_M);
	} else {
		ci::gl::Texture::Format	fmt;
		if((h.mFlags&ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0) {
			fmt.enableMipmapping(true);
			fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		} 
		h.mTexture = ci::gl::Texture(out.mSurface, fmt);

		// If we ran out of memory, try again! why not!
		if(glGetError() == GL_OUT_OF_MEMORY) {
			if(out.mNumberTries < 2){
				DS_LOG_ERROR_M("LoadImageService::onLoadComplete() called on filename: " << out.mKey.mFilename << " received an out of memory error. Image may be too big.", LOAD_IMAGE_LOG_M);
			}
			if(h.mTexture) h.mTexture.reset();

			if(out.mNumberTries >= mMaxLoadTries){
				DS_LOG_WARNING("Gave up loading image for " << loadThread.mOutput.mKey.mFilename << " after " << loadThread.mOutput.mNumberTries << " attempts.");
				loadThread.mOutput.clear();
				out.clear();
			} else {
				mOperationsQueue.push_back(out);
			}
			advanceQueue();
			return;
		}

		DS_REPORT_GL_ERRORS();
	}
	out.clear();
	loadThread.mOutput.clear();

	advanceQueue();
}

void LoadImageService::clear()
{
	mImageResource.clear();
}


LoadImageService::ImageLoadThread::ImageLoadThread(){

}
void LoadImageService::ImageLoadThread::run(){

	mError = true;
	try {

		// If there's a function, then require this image have an alpha channel, because
		// who knows what the function will need. Otherwise let cinder do its thing.
		boost::tribool						alpha = boost::logic::indeterminate;
		if(!mOutput.mIpFunction.empty())	alpha = boost::tribool(true);

		const std::string					fn = ds::Environment::expand(mOutput.mKey.mFilename);
		const Poco::File file(fn);

		if(file.exists()) {
			mOutput.mSurface = ci::Surface8u(ci::loadImage(fn), ci::SurfaceConstraintsDefault(), alpha);
			if(mOutput.mSurface) {
				mOutput.mIpFunction.on(mOutput.mKey.mIpParams, mOutput.mSurface);
				mError = false;
			}
		} else {
			if(mOutput.mNumberTries < 2){
				DS_LOG_WARNING_M("LoadImageService::ImageLoadThread::run() failed. File does not exist: " << mOutput.mKey.mFilename, LOAD_IMAGE_LOG_M);
			}
			mError = true;
		}
	} catch(std::exception const& ex) {

		try{
			// If there's a function, then require this image have an alpha channel, because
			// who knows what the function will need. Otherwise let cinder do its thing.
			boost::tribool						alpha = boost::logic::indeterminate;
			if(!mOutput.mIpFunction.empty())	alpha = boost::tribool(true);

			// Try to load from a url path instead of locally
			mOutput.mSurface = ci::Surface8u(ci::loadImage(ci::loadUrl(mOutput.mKey.mFilename)), ci::SurfaceConstraintsDefault(), alpha);
			if(mOutput.mSurface) {
				mOutput.mIpFunction.on(mOutput.mKey.mIpParams, mOutput.mSurface);
				mError = false;
			} else {
				if(mOutput.mNumberTries < 2){
					DS_LOG_WARNING_M("LoadImageService::ImageLoadThread::run() failed fallback loading. Original exception ex=" << ex.what() << " (file=" << mOutput.mKey.mFilename << ")", LOAD_IMAGE_LOG_M);
				}
				mError = true;
			}
		} catch(std::exception const& extwo){
			if(mOutput.mNumberTries < 2){
				DS_LOG_WARNING_M("LoadImageService::ImageLoadThread::run() failed extwo=" << extwo.what() << " (file=" << mOutput.mKey.mFilename << ")", LOAD_IMAGE_LOG_M);
			}
			mError = true;
		}

		if(mError){
			if(mOutput.mNumberTries < 2){
				DS_LOG_WARNING_M("LoadImageService::ImageLoadThread::run() failed ex=" << ex.what() << " (file=" << mOutput.mKey.mFilename << ")", LOAD_IMAGE_LOG_M);
			}
			mError = true;
		}
	}
}


/**
 * \class ds::ui::LoadImageService::holder
 */
LoadImageService::ImageHolder::ImageHolder()
		: mRefs(0)
		, mError(false)
		, mFlags(0) {
}

/**
 * \class ds::ui::LoadImageService::op
 */
LoadImageService::ImageOperation::ImageOperation()
		: mFlags(0)
		, mNumberTries(0)
{
}

LoadImageService::ImageOperation::ImageOperation(const ImageOperation& o) {
	*this = o;
}

LoadImageService::ImageOperation::ImageOperation(const ImageKey& key, const int flags, const ds::ui::ip::FunctionRef& fn)
		: mKey(key)
		, mFlags(flags)
		, mIpFunction(fn)
		, mNumberTries(0)
{
}

void LoadImageService::ImageOperation::clear() {
	mKey.clear();
	mSurface.reset();
	mFlags = 0;
	mIpFunction.clear();
	mNumberTries = 0;
}

} // namespace ui
} // namespace ds