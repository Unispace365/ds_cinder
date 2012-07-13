#include "ds/ui/service/load_image_service.h"

#include <cinder/ImageIo.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"

namespace {
const ds::BitMask     LOAD_IMAGE_LOG_M = ds::Logger::newModule("load_image");
}

namespace ds {

namespace ui {

/* DS::IMAGE-TOKEN
 ******************************************************************/
ImageToken::ImageToken(LoadImageService& _srv)
	: mSrv(_srv)
{
	init();
}

ImageToken::~ImageToken()
{
	release();
}

bool ImageToken::canAcquire() const
{
	return !mAcquired && !mError;
}

void ImageToken::acquire(const std::string& _filename, const int flags)
{
	if (mAcquired) return;

	if (_filename.empty()) {
		mError = true;
    DS_LOG_WARNING_M("ImageToken: Unable to load image resource (no filename)", LOAD_IMAGE_LOG_M);
		return;
	}

	mAcquired = mSrv.acquire(_filename, flags);
	if (mAcquired) {
		mFilename = _filename;
	}
}

void ImageToken::release()
{
	if (mAcquired) mSrv.release(mFilename);
	init();
}

ci::gl::Texture ImageToken::getImage(float& fade)
{
	if (!mAcquired) return ci::gl::Texture();

	if (!mTexture) {
		return (mTexture = mSrv.getImage(mFilename, fade));
	}

	fade = 1;
	return mTexture;
}

const ci::gl::Texture ImageToken::peekImage(const std::string& filename) const
{
	return mSrv.peekImage(filename);
}

void ImageToken::init()
{
  mFilename.clear();
	mAcquired = false;
	mError = false;
	mTexture.reset();
}

/* DS::LOAD-IMAGE-SERVICE::HOLDER
 ******************************************************************/
LoadImageService::holder::holder()
	: mRefs(0)
	, mError(false)
	, mFlags(0)
{
}

/* DS::LOAD-IMAGE-SERVICE
 ******************************************************************/
LoadImageService::LoadImageService(GlThread& t)
	: GlThreadClient<LoadImageService>(t)
{
	mInput.reserve(64);
	mOutput.reserve(64);
	mTmp.reserve(64);
}

LoadImageService::~LoadImageService()
{
	clear();
}

bool LoadImageService::acquire(const std::string& filename, const int flags)
{
	holder&		h = mImageResource[filename];
	// We have to test multiple conditions here -- if our refs fall below 1 AND we have no
	// current image, then we need to load one in.  But if the refs are > 0, then there's
	// either an image or one's being loaded.  And if there's an image but the refs are < 1,
	// then it's being cached.
	if ((!h.mTexture) && h.mRefs < 1) {
    DS_LOG_INFO_M("ImageService: acquire resource '" << filename << "' flags=" << flags << " refs=" << h.mRefs, LOAD_IMAGE_LOG_M);
		// There's no image, so push on an operation to start one
		{
			Poco::Mutex::ScopedLock		l(mMutex);
			mInput.push_back(op(filename, flags));
 		}
		performOnWorkerThread(&LoadImageService::_load);
	}
	h.mRefs++;
	if ((flags&Image::IMG_CACHE_F) != 0) h.mFlags |= Image::IMG_CACHE_F;

	return true;
}

void LoadImageService::release(const std::string& filename)
{
  // Note:  As far as I can tell, find() always throws an error if the map is empty.
  // Further, I can't even seem to catch the error, so really not sure what's going on there.
  if (mImageResource.empty()) {
    DS_LOG_WARNING_M("LoadImageService::release() called on empty map", LOAD_IMAGE_LOG_M);
    return;
  }

  auto it = mImageResource.find(filename);
  if (it != mImageResource.end()) {
    holder&		h = it->second;
    h.mRefs--;
    // If I'm caching this image, never release it
    if ((h.mFlags&Image::IMG_CACHE_F) == 0 && h.mRefs <= 0) {
      mImageResource.erase(filename);
    }
  } else {
    DS_LOG_WARNING_M("LoadImageService::release() called on filename that doesn't exist (" << filename << ")", LOAD_IMAGE_LOG_M);
  }
}

ci::gl::Texture LoadImageService::getImage(const std::string& filename, float& fade)
{
  // Anytime someone asks for an image, flush out the buffer.
	update();

	holder& h = mImageResource[filename];
	fade = 1;
	return h.mTexture;
}

const ci::gl::Texture LoadImageService::peekImage(const std::string& filename) const
{
	auto it = mImageResource.find(filename);
	if (it != mImageResource.end()) {
		const holder&		h = it->second;
    return h.mTexture;
	}
	return nullptr;
}

bool LoadImageService::peekToken(const std::string& filename, int* flags) const
{
	auto it = mImageResource.find(filename);
	if (it != mImageResource.end()) {
		const holder&		h = it->second;
		if (flags) *flags = h.mFlags;
		return true;
	}
	return false;
}

void LoadImageService::update()
{
	Poco::Mutex::ScopedLock			l(mMutex);
  for (int k=0; k<mOutput.size(); k++) {
		op&							  out = mOutput[k];
		holder&						h = mImageResource[out.mFilename];
		if (h.mTexture) {
#ifdef _DEBUG
			std::cout << "WHHAAAAT?  Duplicate images for id=" << out.mFilename << " refs=" << h.mRefs << std::endl;
#endif
		} else {
      h.mTexture = ci::gl::Texture(out.mSurface);
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
      DS_LOG_INFO_M("LoadImageService::_load() on file (" << top.mFilename << ")", LOAD_IMAGE_LOG_M);
      top.mSurface = ci::Surface32f(ci::loadImage(top.mFilename));
      DS_REPORT_GL_ERRORS();
      if (top.mSurface) {
        // This is to immediately place operations on the out put...
        Poco::Mutex::ScopedLock		l(mMutex);
        mOutput.push_back(op(top));
      }
    } catch (std::exception const& ex) {
      DS_LOG_WARNING_M("LoadImageService::_load() failed ex=" << ex.what(), LOAD_IMAGE_LOG_M);
    }
    top.clear();
		DS_REPORT_GL_ERRORS();
	}
	mTmp.clear();
}

/* DS::LOAD-IMAGE-SERVICE::OP
 ******************************************************************/
LoadImageService::op::op()
	: mFlags(0)
{
}

LoadImageService::op::op(const op& o)
{
	*this = o;
}

LoadImageService::op::op(const std::string& filename, const int flags)
  : mFilename(filename)
  , mFlags(flags)
{
}

void LoadImageService::op::clear()
{
  mFilename.clear();
  mSurface.reset();
  mFlags = 0;
}

} // namespace ui

} // namespace ds