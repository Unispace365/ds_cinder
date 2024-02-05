#include "stdafx.h"

#include "image.h"

#include <map>
#include <utility>

#include <cinder/ImageIo.h>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/logger.h"
#include "ds/ui/layout/layout_sprite.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/image_meta_data.h"
#include <ds/app/environment.h>

namespace {

// Helpers
bool approxEqual(float a, float b, float epsilon = 1.0e-5f) {
	return fabsf(a - b) < epsilon;
}

bool approxZero(float a, float epsilon = 1.0e-5f) {
	return fabsf(a) < epsilon;
}

} // namespace

namespace {
char BLOB_TYPE = 0;

const ds::ui::DirtyState& IMG_SRC_DIRTY	 = ds::ui::INTERNAL_A_DIRTY;
const ds::ui::DirtyState& IMG_CROP_DIRTY = ds::ui::INTERNAL_B_DIRTY;


const char IMG_SRC_ATT	 = 80;
const char IMG_CROP_ATT	 = 81;
const char RES_FLAGS_ATT = 82;

const std::string CircleCropFrag = R"FRAG(
#version 150

in vec2 position_interpolated;
in vec2 texture_interpolated;
in vec2 extent_interpolated;
in vec4 extra_interpolated;

uniform sampler2D tex0;
uniform bool	  useTexture;
uniform bool	  preMultiply;

in vec2	 TexCoord0;
in vec4	 Color;
out vec4 oColor;

void main() {
	oColor = vec4(1.0, 1.0, 1.0, 1.0);

	if (useTexture) {
		oColor = texture2D(tex0, TexCoord0);
	}

	oColor *= Color;

	if (preMultiply) {
		oColor.r *= oColor.a;
		oColor.g *= oColor.a;
		oColor.b *= oColor.a;
	}

	vec2 circleExtent = vec2(extra_interpolated.z - extra_interpolated.x, extra_interpolated.w - extra_interpolated.y);
	vec2 circleRadius = circleExtent * 0.5;
	vec2 circleCenter =
		vec2((extra_interpolated.x + extra_interpolated.z) * 0.5, (extra_interpolated.y + extra_interpolated.w) * 0.5);
	vec2 delta = position_interpolated - circleCenter;

	// apply the general equation of an ellipse
	float radialDistance = (((delta.x * delta.x) / (circleRadius.x * circleRadius.x)) +
							((delta.y * delta.y) / (circleRadius.y * circleRadius.y)));

	float totalAlpha = 1.0;

	// do this with minimal aliasing
	float fragDelta = fwidth(radialDistance) * 1.1;
	totalAlpha		= 1.0 - smoothstep(1.0 - fragDelta, 1.0, radialDistance);

	oColor.a *= totalAlpha;
}
)FRAG";

const std::string CircleCropVert = R"VERT(
#version 150
out vec2 position_interpolated;
out vec2 texture_interpolated;
out vec4 extra_interpolated;
out vec2 extent_interpolated;

uniform bool useTexture;
uniform vec2 extent;
uniform vec4 extra;
uniform mat4 ciModelMatrix;
uniform mat4 ciModelViewProjection;
uniform vec4 uClipPlane0;
uniform vec4 uClipPlane1;
uniform vec4 uClipPlane2;
uniform vec4 uClipPlane3;

in	vec4 ciPosition;
in	vec2 ciTexCoord0;
in	vec4 ciColor;
out vec2 TexCoord0;
out vec4 Color;

void main() {
	position_interpolated = ciPosition.xy;
	if (useTexture) {
		texture_interpolated = texture_interpolated;
	}

	extent_interpolated = extent;

	// see if extra has non-zero width and height
	if (((extra.z - extra.x) > 0) && ((extra.w - extra.y) > 0)) {
		// specified, so use it
		extra_interpolated = extra;
	} else {
		// use the entire extent
		extra_interpolated = vec4(0, 0, extent.xy);
	}

	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0	= ciTexCoord0;
	Color		= ciColor;

	gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);
	gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);
	gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);
	gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);
}
)VERT";
} // namespace

namespace ds::ui {

using namespace ci;


void Image::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
}

void Image::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<Image>(r); });
}

Image& Image::makeImage(SpriteEngine& e, const std::string& fn, Sprite* parent) {
	return makeAlloc<ds::ui::Image>([&e, &fn]() -> ds::ui::Image* { return new ds::ui::Image(e, fn); }, parent);
}

Image& Image::makeImage(SpriteEngine& e, const ds::Resource& r, Sprite* parent) {
	return makeImage(e, ds::Environment::expand(r.getPortableFilePath()), parent);
}

Image::Image(SpriteEngine& engine)
  : Sprite(engine)
  , mStatusFn(nullptr)
  , mCircleCropped(false)
  , mCircleCropCentered(false)
  , mTextureRef(nullptr) {

	mStatus.mCode		 = Status::STATUS_EMPTY;
	mDrawRect.mOrthoRect = ci::Rectf::zero();
	mDrawRect.mPerspRect = ci::Rectf::zero();
	mBlobType			 = BLOB_TYPE;

	setTransparent(false);
	setUseShaderTexture(true);

	markAsDirty(IMG_SRC_DIRTY);
	markAsDirty(IMG_CROP_DIRTY);

	mLayoutFixedAspect = true;
}

Image::Image(SpriteEngine& engine, const std::string& filename, const int flags)
  : Image(engine) {
	setImageFile(filename, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags)
  : Image(engine) {
	setImageResource(resourceId, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource& resource, const int flags)
  : Image(engine) {
	setImageResource(resource, flags);
}

Image::~Image() {
	mEngine.getLoadImageService().release(mFilename, this);
}

void Image::setImageFile(const std::string& filename, const int flags) {
	if (mFilename == filename && mFlags == flags) {
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

	mEngine.getLoadImageService().acquire(
		mFilename, flags, this,
		[this](ci::gl::TextureRef tex, ci::Rectf coords, const bool error, const std::string& errorMsg) {
			mTextureRef = std::move(tex);
			if (error) {
				mErrorMsg = errorMsg;
				setStatus(Status::STATUS_ERRORED);
			} else {
				// Adjust image coordinates if trimmed.
				if (mFlags & IMG_TRIM_WHITESPACE_F) {
					mResource.setCrop(coords);
					imageChanged();

					// Make sure all parent layouts are updated prior to checking status.
					auto parent = getParent();
					while (parent) {
						auto layout = dynamic_cast<ds::ui::LayoutSprite*>(parent);
						if (layout) layout->runLayout();

						parent = parent->getParent();
					}
				}

				checkStatus();

				if ((mFlags & ds::ui::Image::IMG_SKIP_METADATA_F) && mCircleCropCentered) {
					circleCropAutoCenter();
				}
			}
		});

	if (mCircleCropCentered) {
		circleCropAutoCenter();
	}
}

void Image::setImageResource(const ds::Resource& resource, const int flags) {
	mResource = resource;

	setImageFile(resource.getAbsoluteFilePath(), flags);
}

void Image::setImageResource(const ds::Resource::Id& resourceId, const int flags) {
	if (!resourceId.empty()) {
		mEngine.getResources().get(resourceId, mResource);
	}

	if (!mResource.empty()) {
		setImageResource(mResource, flags);
	} else {
		DS_LOG_WARNING("Image:setImageResource couldn't find the resourceid " << resourceId.mValue);
	}
}

void Image::onUpdateServer(const UpdateParams& up) {
	// checkStatus();
}

void Image::onUpdateClient(const UpdateParams& up) {
	// checkStatus();
}

void Image::drawLocalClient() {
	if (!inBounds() || !isLoaded()) return;

	if (mTextureRef) {

		mTextureRef->bind();
		if (mRenderBatch) {
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
	mFilename	= "";
	mResource	= ds::Resource();
	imageChanged();
}

float Image::getWidthMin() const {
	if (mMinWidth <= 0 && mMinHeight > 0) {
		const float aspect = getWidth() / getHeight();
		return mMinHeight * aspect;
	}
	return mMinWidth;
}

float Image::getWidthMax() const {
	if (mMaxWidth <= 0 && mMaxHeight > 0) {
		const float aspect = getWidth() / getHeight();
		return mMaxHeight * aspect;
	}
	return mMaxWidth;
}

float Image::getHeightMin() const {
	if (mMinHeight <= 0 && mMinWidth > 0) {
		const float aspect = getHeight() / getWidth();
		return mMinWidth * aspect;
	}
	return mMinHeight;
}

float Image::getHeightMax() const {
	if (mMaxHeight <= 0 && mMaxWidth > 0) {
		const float aspect = getHeight() / getWidth();
		return mMaxWidth * aspect;
	}
	return mMaxHeight;
}

void Image::setSize(float width, float height) {
	setSizeAll(width, height, mDepth);
}

void Image::setSizeAll(float width, float height, float depth) {
	setScale(width / getWidth(), height / getHeight());
}

bool Image::isLoaded() const {
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Image::setCircleCrop(bool circleCrop) {
	mCircleCropped = circleCrop;
	if (circleCrop) {
		// switch to crop shader
		mSpriteShader.setShaders(CircleCropVert, CircleCropFrag, "image_circle_crop");
	} else {
		// go back to base shader
		mSpriteShader.setToDefaultShader();
	}

	mNeedsBatchUpdate = true;
	markAsDirty(IMG_CROP_DIRTY);
}

void Image::setCircleCropRect(const ci::Rectf& rect) {
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
	if (scw > sch) {
		setCircleCropRect(ci::Rectf(scw / 2.0f - sch / 2.0f, 0.0f, scw / 2.0f + sch / 2.0f, sch));
	} else {
		setCircleCropRect(ci::Rectf(0.0f, sch / 2.0f - scw / 2.0f, scw, sch / 2.0f + scw / 2.0f));
	}
}

void Image::disableCircleCropCentered() {
	mCircleCropCentered = false;
	setCircleCrop(false);
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn) {
	mStatusFn = fn;

	// In case the image was already loaded (cached or onscreen already), and the callback function gets set after
	// the setImage call, make sure we get the message
	if (mStatus.mCode == Status::STATUS_LOADED && mStatusFn) {
		mStatusFn(mStatus);
	}
}

bool Image::isLoadedPrimary() const {
	return isLoaded();
}

void Image::imageChanged() {
	if (mEngine.getMode() == SpriteEngine::CLIENT_MODE) return;

	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);
	doOnImageUnloaded();


	// Make my size match
	ImageMetaData d;
	if (!(mFlags & ds::ui::Image::IMG_SKIP_METADATA_F) && getMetaData(d) && !d.empty()) {
		/* if (!mResource.empty()) {
			auto crop = mResource.getCrop();
			float width = d.mSize.x * crop.getWidth();
			float height = d.mSize.x * crop.getHeight();
			Sprite::setSizeAll(width, height, mDepth);
		}else{

		}*/
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
		auto filename		  = buf.read<std::string>();
		auto resourceFileName = ds::Environment::expand(buf.read<std::string>());
		auto resource		  = ds::Resource(resourceFileName, ds::Resource::IMAGE_TYPE);
		resource.setWidth(buf.read<float>());
		resource.setHeight(buf.read<float>());
		auto flags = buf.read<int>();

		if (resourceFileName.empty()) {
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
	std::string fn;
	if (!mResource.empty()) {
		if (mResource.getWidth() > 0 && mResource.getHeight() > 0) {
			d.mSize.x = mResource.getWidth() * mResource.getCrop().getWidth();
			d.mSize.y = mResource.getHeight() * mResource.getCrop().getHeight();
			return true;
		}
		fn = mResource.getAbsoluteFilePath();
	} else {
		fn = mFilename;
	}

	if (fn.empty()) return false;

	d = ImageMetaData(fn);
	return !d.empty();
}

void Image::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	if (code != Status::STATUS_ERRORED) mErrorMsg = "";

	mStatus.mCode = code;
	if (mStatusFn) mStatusFn(mStatus);
}

void Image::checkStatus() {
	if (mTextureRef) {
		setStatus(Status::STATUS_LOADED);
		doOnImageLoaded();

		if (mEngine.getMode() != mEngine.CLIENT_MODE) {
			const float prevRealW = getWidth(), prevRealH = getHeight();
			if (prevRealW <= 0 || prevRealH <= 0) {
				Sprite::setSizeAll(static_cast<float>(mDrawRect.mOrthoRect.getWidth()),
								   static_cast<float>(mDrawRect.mOrthoRect.getHeight()), mDepth);
			} else {
				float prevWidth	 = prevRealW * getScale().x;
				float prevHeight = prevRealH * getScale().y;
				Sprite::setSizeAll(static_cast<float>(mDrawRect.mOrthoRect.getWidth()),
								   static_cast<float>(mDrawRect.mOrthoRect.getHeight()), mDepth);
				setSize(prevWidth, prevHeight);
			}
		}
	}
}

void Image::onBuildRenderBatch() {
	if (mDrawRect.mOrthoRect.getWidth() < 1.0f) return;

	auto drawRect = mDrawRect.mOrthoRect;
	if (getPerspective()) drawRect = mDrawRect.mPerspRect;
	if (mCornerRadius > 0.0f) {
		auto theGeom = ci::geom::RoundedRect(drawRect, mCornerRadius * (1.f / getScale().x));
		if (mRenderBatch) {
			mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
		} else {
			mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
		}

	} else {
		ci::vec2 ul = ci::vec2(0.f, 0.f);
		ci::vec2 ur = ci::vec2(1.f, 0.f);
		ci::vec2 lr = ci::vec2(1.f, 1.f);
		ci::vec2 ll = ci::vec2(0.f, 1.f);
		if (!mResource.empty()) {
			auto crop = mResource.getCrop();
			ul		  = crop.getUpperLeft();
			ur		  = crop.getUpperRight();
			lr		  = crop.getLowerRight();
			ll		  = crop.getLowerLeft();

			// Invert y-coordinates if image is not loaded top-down.
			if (!mTextureRef->isTopDown()) {
				std::swap(ul, ll);
				std::swap(ur, lr);
				ul.y = 1.0f - ul.y;
				ur.y = 1.0f - ur.y;
				lr.y = 1.0f - lr.y;
				ll.y = 1.0f - ll.y;
			}
		}
		auto theGeom = ci::geom::Rect(drawRect).texCoords(ll, lr, ur, ul);
		if (mRenderBatch) {
			mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theGeom));
		} else {
			mRenderBatch = ci::gl::Batch::create(theGeom, mSpriteShader.getShader());
		}
	}
}

void Image::doOnImageLoaded() {
	if (mTextureRef) {
		mNeedsBatchUpdate	 = true;
		mDrawRect.mPerspRect = ci::Rectf(0.0f, static_cast<float>(mTextureRef->getHeight()),
										 static_cast<float>(mTextureRef->getWidth()), 0.0f);


		float orthoW = mTextureRef->getWidth();
		float orthoH = mTextureRef->getHeight();
		if (!mResource.empty()) {
			auto crop = mResource.getCrop();
			orthoW	  = orthoW * crop.getWidth();
			orthoH	  = orthoH * crop.getHeight();
		}

		mDrawRect.mOrthoRect = ci::Rectf(0.0f, 0.0f, orthoW, orthoH);
	}

	onImageLoaded();
}

void Image::doOnImageUnloaded() {
	onImageUnloaded();
}

} // namespace ds::ui
