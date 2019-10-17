#include "stdafx.h"

#include "image.h"

#include <map>

#include <cinder/ImageIo.h>

#include "ds/debug/logger.h"
#include "ds/app/blob_reader.h"
#include <ds/app/environment.h>
#include "ds/data/data_buffer.h"
#include "ds/app/blob_registry.h"
#include "ds/util/image_meta_data.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/service/load_image_service.h"


namespace ds {
namespace ui {

using namespace ci;

namespace {
char				BLOB_TYPE			= 0;

const DirtyState&	IMG_SRC_DIRTY		= INTERNAL_A_DIRTY;
const DirtyState&	IMG_CROP_DIRTY		= INTERNAL_B_DIRTY;


const char			IMG_SRC_ATT			= 80;
const char			IMG_CROP_ATT		= 81;
const char          RES_FLAGS_ATT		= 82;

const std::string CircleCropFrag =
"#version 150\n"

"in vec2 position_interpolated; \n"
"in vec2 texture_interpolated; \n"
"in vec2 extent_interpolated; \n"
"in vec4 extra_interpolated; \n"

"uniform sampler2D tex0; \n"
"uniform bool useTexture; \n"
"uniform bool preMultiply; \n"

"in vec2            TexCoord0; \n"
"in vec4            Color; \n"
"out vec4           oColor; \n"

"void main()\n"
"{\n"
"oColor = vec4(1.0, 1.0, 1.0, 1.0); \n"

"	if(useTexture) {\n"
"		oColor = texture2D(tex0, TexCoord0);\n"
"	}\n"

"	oColor *= Color;\n"

"	if(preMultiply) {\n"
"		oColor.r *= oColor.a;\n"
"		oColor.g *= oColor.a;\n"
"		oColor.b *= oColor.a;\n"
"	}\n"

"	vec2 circleExtent = vec2(\n"
"		extra_interpolated.z - extra_interpolated.x,\n"
"		extra_interpolated.w - extra_interpolated.y\n"
"		);\n"
"	vec2 circleRadius = circleExtent * 0.5;\n"
"	vec2 circleCenter = vec2(\n"
"		(extra_interpolated.x + extra_interpolated.z) * 0.5,\n"
"		(extra_interpolated.y + extra_interpolated.w) * 0.5\n"
"		);\n"
"	vec2 delta = position_interpolated - circleCenter;\n"

	// apply the general equation of an ellipse
"	float radialDistance = (\n"
"		((delta.x * delta.x) / (circleRadius.x * circleRadius.x)) +\n"
"		((delta.y * delta.y) / (circleRadius.y * circleRadius.y))\n"
"		);\n"

"	float totalAlpha = 1.0;\n"

	// do this with minimal aliasing
"	float fragDelta = fwidth(radialDistance) * 1.1;\n"
"	totalAlpha = 1.0 - smoothstep(1.0 - fragDelta, 1.0, radialDistance);\n"

"	oColor.a *= totalAlpha;\n"
"}\n";

const std::string CircleCropVert =
"#version 150\n"
"out vec2 			position_interpolated;\n"
"out vec2 			texture_interpolated;\n"
"out vec4 			extra_interpolated;\n"
"out vec2 			extent_interpolated;\n"

"uniform bool 		useTexture;\n"
"uniform vec2 		extent;\n"
"uniform vec4 		extra;\n"
"uniform mat4		ciModelMatrix;\n"
"uniform mat4		ciModelViewProjection;\n"
"uniform vec4		uClipPlane0;\n"
"uniform vec4 		uClipPlane1;\n"
"uniform vec4		uClipPlane2;\n"
"uniform vec4		uClipPlane3;\n"

"in vec4			ciPosition;\n"
"in vec2			ciTexCoord0;\n"
"in vec4			ciColor;\n"
"out vec2			TexCoord0;\n"
"out vec4			Color;\n"

"void main()\n"
"{\n"
"	position_interpolated = ciPosition.xy;\n"
"	if(useTexture) {\n"
"		texture_interpolated = texture_interpolated;\n"
"	}\n"

"	extent_interpolated = extent;\n"

	// see if extra has non-zero width and height
"	if(((extra.z - extra.x) > 0) && ((extra.w - extra.y) > 0)) {\n"
		// specified, so use it
"		extra_interpolated = extra;\n"
"	} else {\n"
		// use the entire extent
"		extra_interpolated = vec4(0, 0, extent.xy);\n"
"	}\n"

"	gl_Position = ciModelViewProjection * ciPosition;\n"
"	TexCoord0 = ciTexCoord0;\n"
"	Color = ciColor;\n"

"	gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);\n"
"	gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);\n"
"	gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);\n"
"	gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);\n"
"}\n"
;
}

void Image::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Image::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Image>(r);});
}

Image& Image::makeImage(SpriteEngine& e, const std::string& fn, Sprite* parent) {
	return makeAlloc<ds::ui::Image>([&e, &fn]()->ds::ui::Image*{ return new ds::ui::Image(e, fn); }, parent);
}

Image& Image::makeImage(SpriteEngine& e, const ds::Resource& r, Sprite* parent) {
	return makeImage(e, ds::Environment::expand(r.getPortableFilePath()), parent);
}

Image::Image(SpriteEngine& engine)
	: Sprite(engine)
	, mStatusFn(nullptr)
	, mCircleCropped(false)
	, mCircleCropCentered(false)
	, mTextureRef(nullptr)
{
	mStatus.mCode = Status::STATUS_EMPTY;
	mDrawRect.mOrthoRect = ci::Rectf::zero();
	mDrawRect.mPerspRect = ci::Rectf::zero();
	mBlobType = BLOB_TYPE;

	setTransparent(false);
	setUseShaderTexture(true);

	markAsDirty(IMG_SRC_DIRTY);
	markAsDirty(IMG_CROP_DIRTY);
	
	mLayoutFixedAspect = true;
}

Image::Image(SpriteEngine& engine, const std::string& filename, const int flags)
	: Image(engine)
{
	setImageFile(filename, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags)
	: Image(engine)
{
	setImageResource(resourceId, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource& resource, const int flags)
	: Image(engine)
{
	setImageResource(resource, flags);
}

Image::~Image() {
	mEngine.getLoadImageService().release(mFilename, this);
}

void Image::setImageFile(const std::string& filename, const int flags) {
	if(mFilename == filename && mFlags == flags) {
		return;
	}

	mEngine.getLoadImageService().release(mFilename, this);

	if (filename.find("http") == 0) {
		mFilename = filename;
	} else {
		mFilename = ds::Environment::expand(filename);
	}

	mFlags = flags;

	imageChanged();

	mEngine.getLoadImageService().acquire(mFilename, flags, this, [this](ci::gl::TextureRef tex, const bool error, const std::string& errorMsg) {
		mTextureRef = tex;
		if(error) {
			mErrorMsg = errorMsg;
			setStatus(Status::STATUS_EMPTY);
		} else {
			checkStatus();
		}
	});

	if (mCircleCropCentered) {
		circleCropAutoCenter();
	}
}

void Image::setImageResource(const ds::Resource& r, const int flags) {
	mResource = r;

	setImageFile(r.getAbsoluteFilePath(), flags);
}

void Image::setImageResource(const ds::Resource::Id& rid, const int flags) {
	if(!rid.empty()) {
		mEngine.getResources().get(rid, mResource);
	}

	if(!mResource.empty()) {
		setImageResource(mResource, flags);
	} else {
		DS_LOG_WARNING("Image:setImageResource couldn't find the resourceid " << rid.mValue);
	}
}

void Image::onUpdateServer(const UpdateParams& up){
	//checkStatus();
}

void Image::onUpdateClient(const UpdateParams& up){
	//checkStatus();
}

void Image::drawLocalClient(){
	if (!inBounds() || !isLoaded()) return;

	if (mTextureRef){

		mTextureRef->bind();
		if(mRenderBatch){
			mRenderBatch->draw();
		} else {
			const ci::Rectf& useRect = (getPerspective() ? mDrawRect.mPerspRect : mDrawRect.mOrthoRect);
			ci::gl::drawSolidRect(useRect);
		}

		mTextureRef->unbind();
	}
}

void Image::clearImage() {
	mEngine.getLoadImageService().release(mFilename, this);
	mTextureRef = nullptr;
	mFilename = "";
	mResource = ds::Resource();
	imageChanged();
}

void Image::setSize(float width, float height) {
	setSizeAll(width, height, mDepth);
}

void Image::setSizeAll( float width, float height, float depth ){
	setScale( width / getWidth(), height / getHeight() );
}

bool Image::isLoaded() const {
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Image::setCircleCrop(bool circleCrop){
	mCircleCropped = circleCrop;
	if(circleCrop){
		// switch to crop shader
		mSpriteShader.setShaders(CircleCropVert, CircleCropFrag, "image_circle_crop");
	} else {
		// go back to base shader
		mSpriteShader.setToDefaultShader();
	}

	mNeedsBatchUpdate = true;
	markAsDirty(IMG_CROP_DIRTY);
}

void Image::setCircleCropRect(const ci::Rectf& rect){
	markAsDirty(IMG_CROP_DIRTY);
	mShaderExtraData.x = rect.x1;
	mShaderExtraData.y = rect.y1;
	mShaderExtraData.z = rect.x2;
	mShaderExtraData.w = rect.y2;
}

void Image::cicleCropAutoCenter() {
	circleCropAutoCenter();
}

void Image::circleCropAutoCenter() {
	mCircleCropCentered = true;
	setCircleCrop(true);
	const float scw = getWidth();
	const float sch = getHeight();
	if(scw > sch) {
		setCircleCropRect(ci::Rectf(scw / 2.0f - sch / 2.0f, 0.0f, scw / 2.0f + sch / 2.0f, sch));
	} else {
		setCircleCropRect(ci::Rectf(0.0f, sch / 2.0f - scw / 2.0f, scw, sch / 2.0f + scw / 2.0f));
	}
}

void Image::disableCircleCropCentered() {
	mCircleCropCentered = false;
	setCircleCrop(false);
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn){
	mStatusFn = fn;

	// In case the image was already loaded (cached or onscreen already), and the callback function gets set after the setImage call, make sure we get the message
	if(mStatus.mCode == Status::STATUS_LOADED && mStatusFn) {
		mStatusFn(mStatus);
	}
}

bool Image::isLoadedPrimary() const {
	return isLoaded();
}

void Image::imageChanged() {
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);
	doOnImageUnloaded();

	// Make my size match
	ImageMetaData		d;
	if(getMetaData(d) && !d.empty()) {
		Sprite::setSizeAll(d.mSize.x, d.mSize.y, mDepth);
	} else {
		// Metadata not found, reset all internal states
		ds::ui::Sprite::setSizeAll(0, 0, 1.0f);
		ds::ui::Sprite::setScale(1.0f, 1.0f, 1.0f);
		mDrawRect.mOrthoRect = ci::Rectf::zero();
		mDrawRect.mPerspRect = ci::Rectf::zero();
	}

	onImageChanged();
}

void Image::writeAttributesTo(ds::DataBuffer& buf) {
	Sprite::writeAttributesTo(buf);

	if (mDirty.has(IMG_SRC_DIRTY)) {
		buf.add(IMG_SRC_ATT);
		buf.add(mFilename);
		buf.add(mResource.getPortableFilePath());
		buf.add(mResource.getWidth());
		buf.add(mResource.getHeight());
		buf.add(mFlags);
	}

	if (mDirty.has(IMG_CROP_DIRTY)) {
		buf.add(IMG_CROP_ATT);
		buf.add(mCircleCropped);
		buf.add(mShaderExtraData.x);
		buf.add(mShaderExtraData.y);
		buf.add(mShaderExtraData.z);
		buf.add(mShaderExtraData.w);
	}
}

void Image::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == IMG_SRC_ATT) {
		setStatus(Status::STATUS_EMPTY);
		auto filename = buf.read<std::string>(); 
		auto resourceFileName = ds::Environment::expand(buf.read<std::string>());
		auto resource = ds::Resource(resourceFileName, ds::Resource::IMAGE_TYPE);
		resource.setWidth(buf.read<float>());
		resource.setHeight(buf.read<float>());
		auto flags = buf.read<int>();

		if(resourceFileName.empty()) {
			setImageFile(filename, flags);
		} else {
			setImageResource(resource, flags);
		}
		

	} else if (attributeId == IMG_CROP_ATT) {
		mCircleCropped = buf.read<bool>();
		setCircleCrop(mCircleCropped);
		mShaderExtraData.x = buf.read<float>();
		mShaderExtraData.y = buf.read<float>();
		mShaderExtraData.z = buf.read<float>();
		mShaderExtraData.w = buf.read<float>();
	} else {
		Sprite::readAttributeFrom(attributeId, buf);
	}
}


bool Image::getMetaData(ImageMetaData& d) const {
	std::string	fn;
	if(!mResource.empty()) {
		if(mResource.getWidth() > 0 && mResource.getHeight() > 0) {
			d.mSize.x = mResource.getWidth();
			d.mSize.y = mResource.getHeight();
			return true;
		}
		fn = mResource.getAbsoluteFilePath();
	} else {
		fn = mFilename;
	}

	if(fn.empty()) return false;

	d = ImageMetaData(fn);
	return !d.empty();
}

void Image::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	if(code != Status::STATUS_ERRORED) mErrorMsg = "";

	mStatus.mCode = code;
	if (mStatusFn) mStatusFn(mStatus);
}

void Image::checkStatus() {
	if(mTextureRef) {
		setStatus(Status::STATUS_LOADED);
		doOnImageLoaded();

		if (mEngine.getMode() != mEngine.CLIENT_MODE){
			const float prevRealW = getWidth(), prevRealH = getHeight();
			if (prevRealW <= 0 || prevRealH <= 0) {
				Sprite::setSizeAll(static_cast<float>(mTextureRef->getWidth()), static_cast<float>(mTextureRef->getHeight()), mDepth);
			} else {
				float prevWidth = prevRealW * getScale().x;
				float prevHeight = prevRealH * getScale().y;
				Sprite::setSizeAll(static_cast<float>(mTextureRef->getWidth()), static_cast<float>(mTextureRef->getHeight()), mDepth);
				setSize(prevWidth, prevHeight);
			}
		}
	}
}

void Image::onBuildRenderBatch() {
	if(mDrawRect.mOrthoRect.getWidth() < 1.0f) return;

	auto drawRect = mDrawRect.mOrthoRect;
	if(getPerspective()) drawRect = mDrawRect.mPerspRect;
	if(mCornerRadius > 0.0f){
		auto theGeom = ci::geom::RoundedRect(drawRect, mCornerRadius);
		if(mRenderBatch){
			mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
		} else {
			mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
		}

	} else {
		auto theGeom = ci::geom::Rect(drawRect);
		if(mRenderBatch){
			mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
		} else {
			mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
		}
	}
}

void Image::doOnImageLoaded() {
	if (mTextureRef){
		mNeedsBatchUpdate = true;
		mDrawRect.mPerspRect = ci::Rectf(0.0f, static_cast<float>(mTextureRef->getHeight()), static_cast<float>(mTextureRef->getWidth()), 0.0f);
		mDrawRect.mOrthoRect = ci::Rectf(0.0f, 0.0f, static_cast<float>(mTextureRef->getWidth()), static_cast<float>(mTextureRef->getHeight()));
	}

	onImageLoaded();
}

void Image::doOnImageUnloaded() {
	onImageUnloaded();
}

} // namespace ui
} // namespace ds
