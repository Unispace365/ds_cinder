#include "ds/ui/service/glsl_image_service.h"

#include <cinder/ImageIo.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"

namespace {
const ds::BitMask     GLSL_IMAGE_LOG_M = ds::Logger::newModule("glsl_image");
}

namespace ds {
namespace glsl {

namespace {
const std::string				_IMAGE_SERVICE("ds:glslimg");
}

extern const std::string&		IMAGE_SERVICE(_IMAGE_SERVICE);

/* \class ds::glsl::ImageKey
 */
ImageKey::ImageKey()
		: mW(0)
		, mH(0)
		, mFlags(0) {
}

bool ImageKey::operator==(const ImageKey& o) const {
	if (mW != o.mW || mH != o.mH || mFlags != o.mFlags) return false;
	if (mVertex != o.mVertex) return false;
	if (mFragment != o.mFragment) return false;
	return mUniform == o.mUniform;
}

bool ImageKey::empty() const {
	if (mVertex.empty() || mFragment.empty()) return true;
	if (mW < 1 || mH < 1) return true;
	// I don't technically need to supply any values to the shader, so
	// that doesn't need to be checked.
	return false;
}

void ImageKey::setTo(	const std::string& vertex, const std::string& fragment,
						const ds::gl::Uniform& uniform,
						const int w, const int h, const int flags) {
	mVertex = vertex;
	mFragment = fragment;
	mUniform = uniform;
	mW = w;
	mH = h;
	mFlags = flags;
}

void ImageKey::clear() {
	mVertex.clear();
	mFragment.clear();
	mUniform.clear();
	mW = 0;
	mH = 0;
	mFlags = 0;
}

/* \class ds::glsl::ImageToken
 */
ImageToken::ImageToken(ImageService& srv)
		: mSrv(srv) {
	init();
}

ImageToken::~ImageToken() {
	release();
}

void ImageToken::setTo(const ImageKey& key) {
	if (mKey == key) return;

	release();
	mKey = key;
}

ci::gl::Texture ImageToken::getImage(float& fade) {
	if (mKey.empty()) return nullptr;

	if (!mAcquired) {
		mAcquired = mSrv.acquire(mKey);
		if (!mAcquired) return nullptr;
	}

	if (!mTexture) {
		return (mTexture = mSrv.getImage(mKey, fade));
	}

	fade = 1;
	return mTexture;
}

void ImageToken::init() {
	mKey.clear();
	mAcquired = false;
	mError = false;
	mTexture = ci::gl::Texture();
}

void ImageToken::release() {
	if (mAcquired) mSrv.release(mKey);
	init();
}

/* DS::LOAD-IMAGE-SERVICE
 ******************************************************************/
ImageService::ImageService() {
	mInput.reserve(8);
	mOutput.reserve(8);
	mTmp.reserve(8);
}

ImageService::~ImageService() {
	clear();
}

bool ImageService::acquire(const ImageKey& key) {
	holder*			h = find(key);
	// If there's a holder, then the image either exists or is being generated.
	// If there's no holder, then create one and get the process started.
	if (!h) {
		try {
			mCache.push_back(holder(key));
			h = &(mCache.back());
			{
				Poco::Mutex::ScopedLock		l(mMutex);
				mInput.push_back(op(key));
			}
			renderInput();
		} catch (std::exception&) {
			return false;
		}
	}
	if (!h) return false;

	// Always up the refs
	h->mRefs++;
	return true;
}

void ImageService::release(const ImageKey& key) {
	int				index;
	holder*			h = find(key, &index);
	if (!h) return;
	
	h->mRefs--;
	if (h->mRefs <= 0) {
		mCache.erase(mCache.begin()+index);
	}
}

ci::gl::Texture ImageService::getImage(const ImageKey& key, float& fade) {
	// XXX Move to render engine update cycle -- wait, but why?  This is probably more efficient
	update();

	holder*		h = find(key);
	if (!h) return ci::gl::Texture();
	fade = 1;
	return h->mImg;
}

void ImageService::update() {
	Poco::Mutex::ScopedLock			l(mMutex);
	// Pop off any images that have been placed in the output.
	for (int k=0; k<mOutput.size(); ++k) {
		op&							out = mOutput[k];
		holder*						h = find(out.mKey);
		if (h && out.mImg) {
			h->mImg = out.mImg;
		}
	}
	mOutput.clear();
}

void ImageService::clear() {
	mCache.clear();
}

ImageService::holder* ImageService::find(const ImageKey& key, int* index) {
	for (int k=0; k<mCache.size(); ++k) {
		holder&				h(mCache[k]);
		if (h.mKey == key) {
			if (index) *index = k;
			return &h;
		}
	}
	return nullptr;
}

void ImageService::renderInput() {
	// Pop off the items I need
	mTmp.clear();
	{
		Poco::Mutex::ScopedLock			l(mMutex);
		mInput.swap(mTmp);
	}

if (!mInput.empty()) std::cout << "RENDER size=" << mInput.size() << std::endl;

#if 0
	// Load them all
	vector<op>							outs;
	for (int k=0; k<mTmp.size(); k++) {
		const op&						top = mTmp[k];
		GlTexture*						img = _render(top.mKey, manager);
		if (img) {
			try {
				outs.push_back(op(top.mKey, img));
			} catch (std::exception&) {
				delete img;
			}
		}
	}

	// Push the outs to the output, restore my reusable pixels
	{
		Poco::Mutex::ScopedLock			l(mMutex);
		mOutput.insert(mOutput.end(), outs.begin(), outs.end());
	}
	mTmp.clear();
#endif
}

/* DS::LOAD-IMAGE-SERVICE::HOLDER
 ******************************************************************/
ImageService::holder::holder()
		: mRefs(0)
		, mError(false) {
}

ImageService::holder::holder(const ImageKey& key)
		: mKey(key)
		, mRefs(0)
		, mError(false) {
}

/* DS::LOAD-IMAGE-SERVICE::OP
 ******************************************************************/
ImageService::op::op() {
}

ImageService::op::op(const ImageKey& k)
		: mKey(k) {
}

} // namespace glsl
} // namespace ds