#include "ds/ui/service/load_image_service.h"

#include <cinder/ImageIo.h>
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"

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
		DS_LOG_WARNING_M("ImageToken: Unable to load image resource (no filename)", LOAD_IMAGE_LOG_M);
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
LoadImageService::LoadImageService(GlThread& t, ds::ui::ip::FunctionList& list)
		: GlThreadClient<LoadImageService>(t)
		, mFunctions(list) {
	mInput.reserve(64);
	mOutput.reserve(64);
	mTmp.reserve(64);
}

LoadImageService::~LoadImageService()
{
	clear();
}

bool LoadImageService::acquire(const ImageKey& key, const int flags) {
	holder&		h = mImageResource[key];
	// We have to test multiple conditions here -- if our refs fall below 1 AND we have no
	// current image, then we need to load one in.  But if the refs are > 0, then there's
	// either an image or one's being loaded.  And if there's an image but the refs are < 1,
	// then it's being cached.
	if ((!h.mTexture) && h.mRefs < 1) {
//    DS_LOG_INFO_M("ImageService: acquire resource '" << filename << "' flags=" << flags << " refs=" << h.mRefs, LOAD_IMAGE_LOG_M);
		// There's no image, so push on an operation to start one
		{
			Poco::Mutex::ScopedLock		l(mMutex);
			mInput.push_back(op(key, flags, mFunctions.find(key.mIpKey)));
 		}
		performOnWorkerThread(&LoadImageService::_load);
	}
	h.mRefs++;
	if ((flags&Image::IMG_CACHE_F) != 0) h.mFlags |= Image::IMG_CACHE_F;
	if ((flags&Image::IMG_ENABLE_MIPMAP_F) != 0) h.mFlags |= Image::IMG_ENABLE_MIPMAP_F;

	return true;
}

void LoadImageService::release(const ImageKey& key) {
	// Note:  As far as I can tell, find() always throws an error if the map is empty.
	// Further, I can't even seem to catch the error, so really not sure what's going on there.
	if (mImageResource.empty()) {
		DS_LOG_WARNING_M("LoadImageService::release() called on empty map", LOAD_IMAGE_LOG_M);
		return;
	}

	auto			it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		holder&		h = it->second;
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
	// Anytime someone asks for an image, flush out the buffer.
	update();

	if (mImageResource.empty()) return ci::gl::Texture();
	holder& h = mImageResource[key];
	fade = 1;
	return h.mTexture;
}

const ci::gl::Texture LoadImageService::peekImage(const ImageKey& key) const {
	if (mImageResource.empty()) return ci::gl::Texture();
	auto it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		const holder&		h = it->second;
		return h.mTexture;
	}
	return ci::gl::Texture();
}

bool LoadImageService::peekToken(const ImageKey& key, int* flags) const {
	if (mImageResource.empty()) return false;
	auto it = mImageResource.find(key);
	if (it != mImageResource.end()) {
		const holder&		h = it->second;
		if (flags) *flags = h.mFlags;
		return true;
	}
	return false;
}

void LoadImageService::update() {
	Poco::Mutex::ScopedLock			l(mMutex);
	for (int k=0; k<mOutput.size(); k++) {
		op&							out = mOutput[k];
		holder&						h = mImageResource[out.mKey];
		if (h.mTexture) {
#ifdef _DEBUG
			std::cout << "WHHAAAAT?  Duplicate images for id=" << out.mKey.mFilename << " refs=" << h.mRefs << std::endl;
#endif
		} else {
			ci::gl::Texture::Format	fmt;
			if ((h.mFlags&ds::ui::Image::IMG_ENABLE_MIPMAP_F) != 0) {
				fmt.enableMipmapping(true);
				fmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
			}
			h.mTexture = ci::gl::Texture(out.mSurface, fmt);
			if (glGetError() == GL_OUT_OF_MEMORY) {
				DS_LOG_ERROR_M("LoadImageService::update() called on filename: " << out.mKey.mFilename << " received an out of memory error. Image may be too big.", LOAD_IMAGE_LOG_M);
			}
			DS_REPORT_GL_ERRORS();
		}
		out.clear();
	}
	mOutput.clear();
}

void LoadImageService::clear()
{
	mImageResource.clear();
}

void LoadImageService::_load()
{
	// Pop off the items I need
	mTmp.clear();
	{
		Poco::Mutex::ScopedLock			l(mMutex);
		mInput.swap(mTmp);
	}
	// Load them all
	DS_REPORT_GL_ERRORS();
	for (int k=0; k<mTmp.size(); k++) {
		op&						           top = mTmp[k];
		try {
//			DS_LOG_INFO_M("LoadImageService::_load() on file (" << top.mFilename << ")", LOAD_IMAGE_LOG_M);
			// If there's a function, then require this image have an alpha channel, because
			// who knows what the function will need. Otherwise let cinder do its thing.
			boost::tribool					alpha = boost::logic::indeterminate;
			if (!top.mIpFunction.empty()) alpha = boost::tribool(true);
			const std::string				fn = ds::Environment::expand(top.mKey.mFilename);
			top.mSurface = ci::Surface8u(ci::loadImage(fn), ci::SurfaceConstraintsDefault(), alpha);
			DS_REPORT_GL_ERRORS();
			if (top.mSurface) {
				top.mIpFunction.on(top.mKey.mIpParams, top.mSurface);
				// This is to immediately place operations on the output...
				Poco::Mutex::ScopedLock		l(mMutex);
				mOutput.push_back(op(top));
			}
		} catch (std::exception const& ex) {
			DS_LOG_WARNING_M("LoadImageService::_load() failed ex=" << ex.what() << " (file=" << top.mKey.mFilename << ")", LOAD_IMAGE_LOG_M);
		}
		top.clear();
		DS_REPORT_GL_ERRORS();
	}
	mTmp.clear();
}

/**
 * \class ds::ui::LoadImageService::holder
 */
LoadImageService::holder::holder()
		: mRefs(0)
		, mError(false)
		, mFlags(0) {
}

/**
 * \class ds::ui::LoadImageService::op
 */
LoadImageService::op::op()
		: mFlags(0) {
}

LoadImageService::op::op(const op& o) {
	*this = o;
}

LoadImageService::op::op(const ImageKey& key, const int flags, const ds::ui::ip::FunctionRef& fn)
		: mKey(key)
		, mFlags(flags)
		, mIpFunction(fn) {
}

void LoadImageService::op::clear() {
	mKey.clear();
	mSurface.reset();
	mFlags = 0;
	mIpFunction.clear();
}

} // namespace ui
} // namespace ds