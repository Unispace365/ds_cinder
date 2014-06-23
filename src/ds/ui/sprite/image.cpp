#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/image_meta_data.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE			= 0;

const DirtyState&	IMG_SRC_DIRTY		= INTERNAL_A_DIRTY;

const char			IMG_SRC_ATT			= 80;

const ds::BitMask   SPRITE_LOG			= ds::Logger::newModule("image sprite");
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
	return makeImage(e, r.getAbsoluteFilePath(), parent);
}

Image::Image(SpriteEngine& engine, const int flags)
		: inherited(engine)
		, ImageOwner(engine) {
	init();
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTextuer(true);

	markAsDirty(IMG_SRC_DIRTY);
}

Image::Image(SpriteEngine& engine, const std::string& filename, const int flags)
		: inherited(engine)
		, ImageOwner(engine) {
	init();
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTextuer(true);

	{
		ImageMetaData			atts(filename);
		Sprite::setSizeAll(atts.mSize.x, atts.mSize.y, mDepth);
	}

	markAsDirty(IMG_SRC_DIRTY);

	setImageFile(filename, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags)
		: inherited(engine)
		, ImageOwner(engine) {
	init();
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTextuer(true);

	setImageResource(resourceId, flags);
}

Image::Image(SpriteEngine& engine, const ds::Resource& resource, const int flags)
	: inherited(engine)
	, ImageOwner(engine) {
		init();
		mBlobType = BLOB_TYPE;
		setTransparent(false);
		setUseShaderTextuer(true);

		setImageResource(resource, flags);
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
	if (!inBounds()) return;

	const ci::gl::Texture*		tex = mImageSource.getImage();
	if (!tex) return;

	// Do texture-based initialization.
	// EH: I don't like this at all, why was it done this way? It makes no sense
	// when the app is in client server mode and the server is never loading the image.
	if (mStatus.mCode != Status::STATUS_LOADED) {
		setStatus(Status::STATUS_LOADED);
		const float         prevRealW = getWidth(), prevRealH = getHeight();
		if (prevRealW <= 0 || prevRealH <= 0) {
			Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
		} else {
			float             prevWidth = prevRealW * getScale().x;
			float             prevHeight = prevRealH * getScale().y;
			Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
			setSize(prevWidth, prevHeight);
		}
	}

	tex->bind();
	if (getPerspective())
		ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(tex->getHeight()), static_cast<float>(tex->getWidth()), 0.0f));
	else
		ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight())));
	tex->unbind();
}

void Image::setSizeAll( float width, float height, float depth ) {
	setScale( width / getWidth(), height / getHeight() );
}

#if 1
void Image::loadImage( const std::string &filename ) {
	DS_DBG_CODE(std::cout << "Image::loadImage() is deprecated, use setImageFile()" << std::endl);
	setImageFile(filename);
}

Image& Image::setResourceFilename( const std::string &filename ) {
	DS_DBG_CODE(std::cout << "Image::setResourceFilename() is deprecated, use setImageFile()" << std::endl);
	setImageFile(filename);
	return *this;
}

Image& Image::setResourceId(const ds::Resource::Id& resourceId) {
	DS_DBG_CODE(std::cout << "Image::setResourceId() is deprecated, use setImageResource()" << std::endl);
	setImageResource(resourceId);
	return *this;
}

void Image::clearResource() {
	DS_DBG_CODE(std::cout << "Image::clearResource() is deprecated, use clearImage()" << std::endl);
	clearImage();
}
#endif

bool Image::isLoaded() const {
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(	mEngine.getMode() == mEngine.STANDALONE_MODE,
					"Currently only works in Standalone mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void Image::onImageChanged() {
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);

	// Make my size match
	ImageMetaData		d;
	if (mImageSource.getMetaData(d) && !d.empty()) {
		Sprite::setSizeAll(d.mSize.x, d.mSize.y, mDepth);
	}
}

void Image::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(IMG_SRC_DIRTY)) {
		buf.add(IMG_SRC_ATT);
		mImageSource.writeTo(buf);
	}
}

void Image::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == IMG_SRC_ATT) {
		mImageSource.readFrom(buf);
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

void Image::setSize( float width, float height ) {
	setSizeAll(width, height, mDepth);
}

} // namespace ui
} // namespace ds
