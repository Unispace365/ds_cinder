#include "stdafx.h"

#include "nine_patch.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/data/resource_list.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
char				BLOB_TYPE		= 0;

const DirtyState&	IMG_SRC_DIRTY	= INTERNAL_A_DIRTY;

const char			IMG_SRC_ATT		= 80;

const ds::BitMask	SPRITE_LOG		= ds::Logger::newModule("ninepatch sprite");

const int			CELL_LT			= 0;
const int			CELL_MT			= 1;
const int			CELL_RT			= 2;
const int			CELL_LM			= 3;
const int			CELL_MM			= 4;
const int			CELL_RM			= 5;
const int			CELL_LB			= 6;
const int			CELL_MB			= 7;
const int			CELL_RB			= 8;
}

/**
 * \class ds::ui::NinePatch
 */
void NinePatch::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void NinePatch::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<NinePatch>(r);});
}

NinePatch& NinePatch::makeNinePatch(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::NinePatch>([&e]()->ds::ui::NinePatch*{ return new ds::ui::NinePatch(e); }, parent);
}

NinePatch& NinePatch::makeNinePatch(SpriteEngine& e, const std::string &file, Sprite *parent) {
	return makeAlloc<ds::ui::NinePatch>([&e, &file]()->ds::ui::NinePatch*{
			ds::ui::NinePatch*	s = new ds::ui::NinePatch(e);
			if (s) s->setImageFile(file);
			return s;
		}, parent);
}

NinePatch::NinePatch(SpriteEngine& engine)
		: ds::ui::Sprite(engine)
		, ImageOwner(engine)
		, mSizeDirty(true) {
	init();
	mBlobType = BLOB_TYPE;
	setUseShaderTexture(true);
	setTransparent(false);
}

void NinePatch::onUpdateServer(const UpdateParams& up) {
	if (mStatusDirty) {
		mStatusDirty = false;
		if (mStatusFn) mStatusFn(mStatus);
	}
}

void NinePatch::drawLocalClient() {
	if (!inBounds()) return;

	const ci::gl::TextureRef		tex = mImageSource.getImage();
	if (!tex) return;

	// Probably should have an initialization stage
	setStatus(Status::STATUS_LOADED);
	if (mPatch.empty()) {
		mPatch.buildSources(tex);
		mSizeDirty = true;
//		mPatch.print();
	}
	if (mSizeDirty) {
		mSizeDirty = false;
		mPatch.buildDestinations(getWidth(), getHeight());
//		mPatch.print();
	}

	if (getPerspective()) {
		tex->bind();
		ci::gl::drawSolidRect(ci::Rectf(0.0f, static_cast<float>(tex->getHeight()), static_cast<float>(tex->getWidth()), 0.0f));
		tex->unbind();
	} else {
//			tex->bind();
//      ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture.getWidth()), static_cast<float>(mTexture.getHeight())));
//      ci::gl::drawSolidRect(ci::Rectf(120.0f, 70.0f, static_cast<float>(mTexture.getWidth()-120), static_cast<float>(mTexture.getHeight()-70)));
//			tex->unbind();

//			ci::Area	src(100, 0, mTexture.getWidth(), mTexture.getHeight());
//			ci::gl::draw(*tex, src, ci::Rectf(0.0f, 0.0f, static_cast<float>(mTexture.getWidth()+300), static_cast<float>(mTexture.getHeight())) );

		mPatch.draw(tex);
	}
}

void NinePatch::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(	mEngine.getMode() == mEngine.STANDALONE_MODE,
					"Currently only works in Standalone mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void NinePatch::onSizeChanged() {
	ds::ui::Sprite::onSizeChanged();
	mSizeDirty = true;
}

void NinePatch::onImageChanged() {
	setStatus(Status::STATUS_EMPTY);
	markAsDirty(IMG_SRC_DIRTY);
	mSizeDirty = true;
}

void NinePatch::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if (mDirty.has(IMG_SRC_DIRTY)) {
		buf.add(IMG_SRC_ATT);
		mImageSource.writeTo(buf);
	}
}

void NinePatch::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == IMG_SRC_ATT) {
		mImageSource.readFrom(buf);
	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

void NinePatch::setStatus(const int code)
{
  if (code == mStatus.mCode) return;

  mStatus.mCode = code;
  mStatusDirty = true;
}

void NinePatch::init()
{
  mStatus.mCode = Status::STATUS_EMPTY;
  mStatusDirty = false;
  mStatusFn = nullptr;
}

/**
 * \class ds::ui::NinePatch::Cell
 */
NinePatch::Cell::Cell()
	: mIsValid(false)
{
}

ci::vec2 NinePatch::Cell::size() const
{
	ci::vec2		ans(0.0f, 0.0f);
	if (mIsValid) {
		ans.x = static_cast<float>(mSrc.getWidth());
		ans.y = static_cast<float>(mSrc.getHeight());
	}
	return ans;
}

void NinePatch::Cell::draw(const ci::gl::TextureRef tex)
{
	if (!mIsValid) return;

	ci::gl::draw(tex, mSrc, mDst);
}

void NinePatch::Cell::print(const int tabs) const
{
	for (int tab=0; tab<tabs; ++tab) std::cout << "\t";
	if (mIsValid) {
		std::cout << "src (" << mSrc << ") dst (" << mDst << ")" << std::endl;
	} else {
		std::cout << "invalid" << std::endl;
	}
}

/**
 * \class ds::ui::NinePatch::Patch
 */
NinePatch::Patch::Patch()
	: mEmpty(true)
{
}

void NinePatch::Patch::clear()
{
	mEmpty = true;
}

bool NinePatch::Patch::empty() const
{
	return mEmpty;
}

void NinePatch::Patch::buildSources(ci::gl::TextureRef tex)
{
	mEmpty = false;
	for (int k=0; k<CELL_SIZE; ++k) mCell[k].mIsValid = false;
	if (!tex) return;

	// Really just need the left and top rows of pixels, so this could
	// be more efficient.
	int					stretchX_start = tex->getWidth()/2,
						stretchY_start = (tex->getHeight()/2);
	int					stretchX_end = stretchX_start,
						stretchY_end = (stretchY_start);
	int					l = 0, t = 0, r = tex->getWidth(), b = tex->getHeight();

// jus playin
//stretchX_start = (int)(tex.getWidth()*0.35f);
//stretchX_end = (int)(tex.getWidth()*0.65f);

	// First do the four corners, which don't stretch

	// LEFT TOP CELL
	if (stretchX_start > l) {
		mCell[CELL_LT].mIsValid = true;
		mCell[CELL_LT].mSrc = ci::Area(l, t, stretchX_start-1, stretchY_start-1);
	}
	// RIGHT TOP CELL
	if (stretchX_end < tex->getWidth()) {
		mCell[CELL_RT].mIsValid = true;
		mCell[CELL_RT].mSrc = ci::Area(stretchX_end+1, t, r, stretchY_start-1);
	}
	// LEFT BOTTOM CELL
	if (stretchX_start > l) {
		mCell[CELL_LB].mIsValid = true;
		mCell[CELL_LB].mSrc = ci::Area(l, stretchY_end+1, stretchX_start-1, b);
	}
	// RIGHT BOTTOM CELL
	if (stretchX_end < tex->getWidth()) {
		mCell[CELL_RB].mIsValid = true;
		mCell[CELL_RB].mSrc = ci::Area(stretchX_end+1, stretchY_end+1, r, b);
	}

	// Interior stretchy cells, based on the corners

	// MIDDLE TOP CELL
	mCell[CELL_MT].mIsValid = true;
	mCell[CELL_MT].mSrc = ci::Area(	stretchX_start, mCell[CELL_LT].mSrc.y1,
									stretchX_end, mCell[CELL_LT].mSrc.y2);
	// LEFT MIDDLE CELL
	mCell[CELL_LM].mIsValid = true;
	mCell[CELL_LM].mSrc = ci::Area(	mCell[CELL_LT].mSrc.x1, stretchY_start,
									mCell[CELL_LT].mSrc.x2, stretchY_end);
	// RIGHT MIDDLE CELL
	mCell[CELL_RM].mIsValid = true;
	mCell[CELL_RM].mSrc = ci::Area(	mCell[CELL_RT].mSrc.x1, mCell[CELL_LM].mSrc.y1,
									mCell[CELL_RT].mSrc.x2, mCell[CELL_LM].mSrc.y2);
	// MIDDLE BOTTOM CELL
	mCell[CELL_MB].mIsValid = true;
	mCell[CELL_MB].mSrc = ci::Area(	stretchX_start, mCell[CELL_LB].mSrc.y1,
									stretchX_end, mCell[CELL_LB].mSrc.y2);
	// MIDDLE MIDDLE CELL
	mCell[CELL_MM].mIsValid = true;
	mCell[CELL_MM].mSrc = ci::Area(	mCell[CELL_MT].mSrc.x1, mCell[CELL_LM].mSrc.y1,
									mCell[CELL_MT].mSrc.x2, mCell[CELL_LM].mSrc.y2);
}

void NinePatch::Patch::buildDestinations(const float width, const float height) {
	// Corners first, which don't stretch

	// LEFT TOP CELL
	if (mCell[CELL_LT].mIsValid) {
		const ci::vec2	size = mCell[CELL_LT].size();
		mCell[CELL_LT].mDst = ci::Rectf(0.0f, 0.0f, size.x, size.y);
	}
	// RIGHT TOP CELL
	if (mCell[CELL_RT].mIsValid) {
		const ci::vec2	size = mCell[CELL_RT].size();
		mCell[CELL_RT].mDst = ci::Rectf(width - size.x, 0.0f, width, size.y);
	}
	// LEFT BOTTOM CELL
	if (mCell[CELL_LB].mIsValid) {
		const ci::vec2	size = mCell[CELL_LB].size();
		mCell[CELL_LB].mDst = ci::Rectf(0.0f, height-size.y, size.x, height);
	}
	// RIGHT BOTTOM CELL
	if (mCell[CELL_RB].mIsValid) {
		const ci::vec2	size = mCell[CELL_RB].size();
		mCell[CELL_RB].mDst = ci::Rectf(width - size.x, height-size.y, width, height);
	}

	// Now adjust for the case where the dest is smaller than the source
	const float			src_w = static_cast<float>(mCell[CELL_RT].mSrc.getX2()),
						src_h = static_cast<float>(mCell[CELL_RB].mSrc.getY2());
	if (height < src_h) {
		const float		half = floorf(height/2.0f);
		mCell[CELL_LT].mDst.y2 = half-1.0f;
		mCell[CELL_RT].mDst.y2 = mCell[CELL_LT].mDst.y2;
		mCell[CELL_LB].mDst.y1 = half+1.0f;
		mCell[CELL_RB].mDst.y1 = mCell[CELL_LB].mDst.y1;
	}
	if (width < src_w) {
		const float		half = floorf(width/2.0f);
		mCell[CELL_LT].mDst.x2 = half-1.0f;
		mCell[CELL_LB].mDst.x2 = mCell[CELL_LT].mDst.x2;
		mCell[CELL_RT].mDst.x1 = half+1.0f;
		mCell[CELL_RB].mDst.x1 = mCell[CELL_RT].mDst.x1;
	}

	// Interior stretchy cells, based on the corners

	// MIDDLE TOP CELL
	if (mCell[CELL_MT].mIsValid) {
		const Cell&		lt(mCell[CELL_LT]);
		mCell[CELL_MT].mDst = ci::Rectf(lt.size().x, lt.mDst.y1, width - mCell[CELL_RT].size().x, lt.mDst.y2);
	}
	// LEFT MIDDLE CELL
	mCell[CELL_LM].mDst = ci::Rectf(mCell[CELL_LT].mDst.x1, mCell[CELL_LT].mDst.y2,
																	mCell[CELL_LT].mDst.x2, mCell[CELL_LB].mDst.y1);
	// RIGHT MIDDLE CELL
	mCell[CELL_RM].mDst = ci::Rectf(mCell[CELL_RT].mDst.x1, mCell[CELL_RT].mDst.y2,
																	mCell[CELL_RT].mDst.x2, mCell[CELL_RB].mDst.y1);
	// MIDDLE BOTTOM CELL
	mCell[CELL_MB].mDst = ci::Rectf(mCell[CELL_LB].mDst.x2, mCell[CELL_LB].mDst.y1,
																	mCell[CELL_RB].mDst.x1, mCell[CELL_LB].mDst.y2);
	// MIDDLE MIDDLE CELL
	mCell[CELL_MM].mDst = ci::Rectf(mCell[CELL_MB].mDst.x1, mCell[CELL_LM].mDst.y1,
																	mCell[CELL_MB].mDst.x2, mCell[CELL_LM].mDst.y2);
}

void NinePatch::Patch::draw(const ci::gl::TextureRef tex)
{
	if (mEmpty) return;
	for (int k=0; k<CELL_SIZE; ++k) mCell[k].draw(tex);
}

void NinePatch::Patch::print(const int tabs) const
{
	for (int tab=0; tab<tabs; ++tab) std::cout << "\t";
	std::cout << "NinePatch" << std::endl;

	for (int k=0; k<CELL_SIZE; ++k) {
		for (int tab=0; tab<tabs+1; ++tab) std::cout << "\t";
		std::cout << k << ": ";
		mCell[k].print(0);
	}
}

} // namespace ui
} // namespace ds
