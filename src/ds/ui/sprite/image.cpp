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
	return makeImage(e, r.getPortableFilePath(), parent);
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
	checkStatus();
}

void Image::drawLocalClient() {
	if (!inBounds() || !isLoaded()) return;

	if (auto tex = mImageSource.getImage())
	{
		if (getPerspective()) ci::gl::draw(*tex, mDrawRect.mPerspRect);
		else ci::gl::draw(*tex, mDrawRect.mOrthoRect);
	}
}

void Image::setSizeAll( float width, float height, float depth ) {
	setScale( width / getWidth(), height / getHeight() );
}

bool Image::isLoaded() const {
	return mStatus.mCode == Status::STATUS_LOADED;
}

void Image::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.STANDALONE_MODE,
		"Currently only works in Standalone mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void Image::onImageChanged() {
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);
	doOnImageUnloaded();

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
	if (mStatusFn) mStatusFn(mStatus);
}

void Image::checkStatus()
{
	if (mImageSource.getImage() && !isLoaded())
	{
		auto tex = mImageSource.getImage();
		setStatus(Status::STATUS_LOADED);
		doOnImageLoaded();
		const float         prevRealW = getWidth(), prevRealH = getHeight();
		if (prevRealW <= 0 || prevRealH <= 0) {
			Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
		}
		else {
			float             prevWidth = prevRealW * getScale().x;
			float             prevHeight = prevRealH * getScale().y;
			Sprite::setSizeAll(static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()), mDepth);
			setSize(prevWidth, prevHeight);
		}
	}
}

void Image::doOnImageLoaded()
{
	if (auto tex = mImageSource.getImage())
	{
		mDrawRect.mPerspRect = ci::Rectf(0.0f, static_cast<float>(tex->getHeight()), static_cast<float>(tex->getWidth()), 0.0f);
		mDrawRect.mOrthoRect = ci::Rectf(0.0f, 0.0f, static_cast<float>(tex->getWidth()), static_cast<float>(tex->getHeight()));
	}

	onImageLoaded();
}

void Image::doOnImageUnloaded()
{
	onImageUnloaded();
}

void Image::init() {
	mStatus.mCode = Status::STATUS_EMPTY;
	mStatusFn = nullptr;
	mDrawRect.mOrthoRect = ci::Rectf::zero();
	mDrawRect.mPerspRect = ci::Rectf::zero();
}

void Image::setSize( float width, float height ) {
	setSizeAll(width, height, mDepth);
}

} // namespace ui
} // namespace ds
