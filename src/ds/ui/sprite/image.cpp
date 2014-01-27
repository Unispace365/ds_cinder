#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/resource_list.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/file_meta_data.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE			= 0;

const DirtyState&	RES_ID_DIRTY		= INTERNAL_A_DIRTY;
const DirtyState&	RES_FN_DIRTY		= INTERNAL_B_DIRTY;
const DirtyState&	FLAGS_DIRTY			= INTERNAL_C_DIRTY;

const char			RES_ID_ATT			= 80;
const char			RES_FN_ATT			= 81;
const char			FLAGS_ATT			= 82;

const ds::BitMask   SPRITE_LOG			= ds::Logger::newModule("image sprite");
}

void Image::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Image::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Image>(r);});
}

Image& Image::makeImage(SpriteEngine& e, const ds::Resource& r, Sprite* parent) {
	return makeAlloc<ds::ui::Image>([&e, &r]()->ds::ui::Image*{ return new ds::ui::Image(e, r.getAbsoluteFilePath()); }, parent);
}

Image::Image(SpriteEngine& engine, const int flags)
		: inherited(engine)
		, mImageService(engine.getLoadImageService())
		, mImageToken(mImageService)
		, mFlags(flags) {
	init();
	mBlobType = BLOB_TYPE;
	setUseShaderTextuer(true);
	setTransparent(false);

	if (mFlags != 0) markAsDirty(FLAGS_DIRTY);
}

Image::Image(SpriteEngine& engine, const std::string &filename, const int flags)
		: inherited(engine)
		, mImageService(engine.getLoadImageService())
		, mImageToken(mImageService)
		, mFlags(flags)
		, mResourceFn(filename) {
	init();
	mBlobType = BLOB_TYPE;
	setUseShaderTextuer(true);

	{
		ImageFileAtts			atts(filename);
		Sprite::setSizeAll(atts.mSize.x, atts.mSize.y, mDepth);
	}

	setTransparent(false);
	markAsDirty(RES_FN_DIRTY);

	if (mFlags != 0) markAsDirty(FLAGS_DIRTY);
}

Image::Image( SpriteEngine& engine, const ds::Resource::Id &resourceId, const int flags)
		: inherited(engine)
		, mImageService(engine.getLoadImageService())
		, mImageToken(mImageService)
		, mFlags(flags)
		, mResourceId(resourceId) {
	init();
	mBlobType = BLOB_TYPE;
	setUseShaderTextuer(true);

	ds::Resource            res;
	if (engine.getResources().get(resourceId, res)) {
		Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
		mResourceFn = res.getAbsoluteFilePath();
	}
	setTransparent(false);
	markAsDirty(RES_ID_DIRTY);

	if (mFlags != 0) markAsDirty(FLAGS_DIRTY);
}

Image::~Image() {
}

void Image::updateServer(const UpdateParams& up) {
	inherited::updateServer(up);

	if (mStatusDirty) {
		mStatusDirty = false;
		if (mStatusFn) mStatusFn(mStatus);
	}
}

void Image::drawLocalClient() {
	if (!inBounds())
		return;

	if (!mTexture) {
		// XXX Do bounds check here
		if (mImageToken.canAcquire()) { // && intersectsLocalScreen){
			requestImage();
		}
		float         fade;
		mTexture = mImageToken.getImage(fade);
		// Keep up the bounds
		if (mTexture) {
			setStatus(Status::STATUS_LOADED);
			const float         prevRealW = getWidth(), prevRealH = getHeight();
			if (prevRealW <= 0 || prevRealH <= 0) {
				Sprite::setSizeAll(static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight()), mDepth);
			} else {
				float             prevWidth = prevRealW * getScale().x;
				float             prevHeight = prevRealH * getScale().y;
				Sprite::setSizeAll(static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight()), mDepth);
				setSize(prevWidth, prevHeight);
			}
		}
	}

	if (mTexture) {
		mTexture.bind();
		if (getPerspective())
			ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(mTexture.getHeight()), static_cast<float>(mTexture.getWidth()), 0.0f));
		else
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight())));
		mTexture.unbind();
	}
}

void Image::setSizeAll( float width, float height, float depth ) {
	setScale( width / getWidth(), height / getHeight() );
}

Image& Image::setResourceFilename( const std::string &filename ) {
	clearResource();
	mResourceFn = filename;
	setStatus(Status::STATUS_EMPTY);

	if (!filename.empty()) {
		ImageFileAtts			atts(filename);
		Sprite::setSizeAll(atts.mSize.x, atts.mSize.y, mDepth);
	}
	return *this;
}

void Image::loadImage( const std::string &filename ) {
	setResourceFilename(filename);
}

void Image::clearResource() {
	mTexture.reset();
	mResourceFn.clear();
	mImageToken.release();
	setStatus(Status::STATUS_EMPTY);
}

void Image::requestImage() {
	if (mResourceFn.empty()) return;

	mImageToken.acquire(mResourceFn, mFlags);
}

bool Image::isLoaded() const {
	return mTexture;
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void Image::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(RES_ID_DIRTY)) {
		buf.add(RES_ID_ATT);
		mResourceId.writeTo(buf);
	}
	if (mDirty.has(RES_FN_DIRTY)) {
		buf.add(RES_FN_ATT);
		buf.add(mResourceFn);
	}
	if (mDirty.has(FLAGS_DIRTY)) {
		buf.add(FLAGS_ATT);
		buf.add(mFlags);
	}
}

void Image::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == RES_ID_ATT) {
		mResourceId.readFrom(buf);
	} else if (attributeId == RES_FN_ATT) {
		mResourceFn = buf.read<std::string>();
	} else if (attributeId == FLAGS_ATT) {
		mFlags = buf.read<int>();
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void Image::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusDirty = true;
}

void Image::init() {
	mStatus.mCode = Status::STATUS_EMPTY;
	mStatusDirty = false;
	mStatusFn = nullptr;
}

Image &Image::setResourceId( const ds::Resource::Id &resourceId ) {
	clearResource();
	ds::Resource            res;
	if (mEngine.getResources().get(resourceId, res)) {
		Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
		mTexture.reset();
		mResourceFn = res.getAbsoluteFilePath();
		setStatus(Status::STATUS_EMPTY);
	}
	markAsDirty(RES_ID_DIRTY);

	// XXX This should check to see if I'm in client mode and only
	// load it then. (or the service should be empty in server mode).
	// Also, ideally this would happen in drawClient(), but that won't
	// be reached if the sprite is not visible, negating a lot of the value.
	if ((mFlags&IMG_PRELOAD_F) != 0) {
		if (mImageToken.canAcquire()) {
			requestImage();
		}
	}

	return *this;
}

void Image::setSize( float width, float height ) {
	setSizeAll(width, height, mDepth);
}

} // namespace ui
} // namespace ds
