#include "ds/ui/service/glsl_image_service.h"

#include <cinder/Camera.h>
#include <cinder/ImageIo.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/fbo/auto_fbo.h"
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

ci::gl::TextureRef ImageToken::getImage(float& fade) {
	if (mKey.empty()) return nullptr;

	if (!mAcquired) {
		mAcquired = mSrv.acquire(mKey);
		if (!mAcquired) return nullptr;
	}

	if (!mTextureRef) {
		return (mTextureRef = mSrv.getImage(mKey, fade));
	}

	fade = 1;
	return mTextureRef;
}

void ImageToken::init() {
	mKey.clear();
	mAcquired = false;
	mError = false;
	mTextureRef = nullptr;
}

void ImageToken::release() {
	if (mAcquired) mSrv.release(mKey);
	init();
}

/* DS::LOAD-IMAGE-SERVICE
 ******************************************************************/
ImageService::ImageService(ds::ui::SpriteEngine& e)
		: mEngine(e) {
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

ci::gl::TextureRef ImageService::getImage(const ImageKey& key, float& fade) {
	// XXX Move to render engine update cycle -- wait, but why?  This is probably more efficient
	update();

	holder*		h = find(key);
	if (!h) return nullptr;
	fade = 1;
	return h->mImgRef;
}

void ImageService::update() {
	Poco::Mutex::ScopedLock			l(mMutex);
	// Pop off any images that have been placed in the output.
	for (int k=0; k<mOutput.size(); ++k) {
		op&							out = mOutput[k];
		holder*						h = find(out.mKey);
		if(h && out.mImgRef) {
			h->mImgRef = out.mImgRef;
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
	// Pop off the items I need. This design is a hold over from
	// the original attempt to make this run in a separate thread.
	mTmp.clear();
	{
		Poco::Mutex::ScopedLock			l(mMutex);
		mInput.swap(mTmp);
	}
	std::vector<op>						outs;
	for (auto it=mTmp.begin(), end=mTmp.end(); it!=end; ++it) {
		op&								top(*it);
		try {
			renderInput(top);
			if(top.mImgRef) outs.push_back(top);
		} catch (std::exception const&) {
		}
	}
	mTmp.clear();
	// Push the outs to the output
	{
		Poco::Mutex::ScopedLock			l(mMutex);
		mOutput.insert(mOutput.end(), outs.begin(), outs.end());
	}
}

void ImageService::renderInput(op& input) {
	input.mImgRef = nullptr;
	// Load the shader -- no shader, no output
	ci::gl::GlslProgRef shader = ci::gl::GlslProg::create(ci::loadFile(input.mKey.getVertex()), ci::loadFile(input.mKey.getFragment()));
	if (!shader) return;

	// Set up the texture
	const int w = input.mKey.getWidth(), h = input.mKey.getHeight();
	if(w < 1 || h < 1) return;
	input.mImgRef = ci::gl::Texture::create(w, h);
	if (!input.mImgRef) return;

	ds::ui::AutoFbo					afbo(mEngine, input.mImgRef);
	{
		afbo.mFbo->offsetViewport(0, 0);
		ci::CameraOrtho camera;
		camera.setOrtho(0.0f, static_cast<float>(afbo.mFbo->getWidth()), static_cast<float>(afbo.mFbo->getHeight()), 0.0f, -1.0f, 1.0f);

		ci::gl::pushMatrices();
		ci::gl::setMatrices(camera);
		ci::gl::disableAlphaBlending();
		ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

		shader->bind();
		shader->uniform("tex0", 0);
		input.mKey.getUnifom().applyTo(shader);
		ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));
		ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)));

		ci::gl::bindStockShader(ci::gl::ShaderDef().color());
		ci::gl::popMatrices();
	}
	ci::gl::popMatrices();
	// SaveCamera should be handling this.
//	mEngine.setCamera();
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
