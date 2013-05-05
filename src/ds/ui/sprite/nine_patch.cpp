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
#include "ds/util/file_name_parser.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
char                BLOB_TYPE         = 0;

const DirtyState&   IMG_SRC_DIRTY 	  = INTERNAL_A_DIRTY;

const char          IMG_SRC_ATT				= 80;

const ds::BitMask   SPRITE_LOG        = ds::Logger::newModule("ninepatch sprite");

const int						CELL_LT						= 0;
const int						CELL_MT						= 1;
const int						CELL_RT						= 2;
const int						CELL_LM						= 3;
const int						CELL_MM						= 4;
const int						CELL_RM						= 5;
const int						CELL_LB						= 6;
const int						CELL_MB						= 7;
const int						CELL_RB						= 8;
}

/**
 * \class ds::ui::NinePatch
 */
void NinePatch::installAsServer(ds::BlobRegistry& registry)
{
  BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void NinePatch::installAsClient(ds::BlobRegistry& registry)
{
  BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<NinePatch>(r);});
}

NinePatch::NinePatch(SpriteEngine& engine)
    : mImageSource(engine)
		, inherited(engine)
{
  init();
  mBlobType = BLOB_TYPE;
  setUseShaderTextuer(true);
  setTransparent(false);
}

NinePatch& NinePatch::setImage(const ImageSource& src)
{
	mImageSource.setSource(src);
	setStatus(Status::STATUS_EMPTY);
  markAsDirty(IMG_SRC_DIRTY);
	return *this;
}

void NinePatch::clearImage()
{
  mImageSource.clear();
  setStatus(Status::STATUS_EMPTY);
  markAsDirty(IMG_SRC_DIRTY);
}

void NinePatch::updateServer(const UpdateParams& up)
{
  inherited::updateServer(up);

  if (mStatusDirty) {
    mStatusDirty = false;
    if (mStatusFn) mStatusFn(mStatus);
  }
}

void NinePatch::drawLocalClient()
{
  if (!inBounds())
    return;

	const ci::gl::Texture*		tex = mImageSource.getImage();
	if (!tex) return;

	// Probably should have an initialization stage
	setStatus(Status::STATUS_LOADED);
	if (mPatch.empty()) {
		mPatch.buildSources(*tex);
		// XXX Need a flag to rebuild when size changes.
		mPatch.buildDestinations(getWidth(), getHeight());
		mPatch.print();
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

		mPatch.draw(*tex);
	}
}

void NinePatch::setStatusCallback(const std::function<void(const Status&)>& fn)
{
  DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
  mStatusFn = fn;
}

void NinePatch::writeAttributesTo(ds::DataBuffer& buf)
{
  inherited::writeAttributesTo(buf);

	if (mDirty.has(IMG_SRC_DIRTY)) {
    buf.add(IMG_SRC_ATT);
    mImageSource.writeTo(buf);
  }
}

void NinePatch::readAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
    if (attributeId == IMG_SRC_ATT) {
      mImageSource.readFrom(buf);
    } else {
      inherited::readAttributeFrom(attributeId, buf);
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

ci::Vec2f NinePatch::Cell::size() const
{
	ci::Vec2f		ans(0.0f, 0.0f);
	if (mIsValid) {
		ans.x = static_cast<float>(mSrc.getWidth());
		ans.y = static_cast<float>(mSrc.getHeight());
	}
	return ans;
}

void NinePatch::Cell::draw(const ci::gl::Texture& tex)
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

void NinePatch::Patch::buildSources(ci::gl::Texture tex)
{
	mEmpty = false;
	for (int k=0; k<CELL_SIZE; ++k) mCell[k].mIsValid = false;
  if (!tex) return;

	// Really just need the left and top rows of pixels, so this could
	// be more efficient.
	ci::Surface8u				s(tex);
	int									stretchX_start = tex.getWidth()/2,
											stretchY_start = tex.getHeight()/2;
	int									stretchX_end = stretchX_start,
											stretchY_end = stretchY_start;
	int									l = 0, t = 0, r = tex.getWidth()-1, b = tex.getHeight()-1;
// jus playin
//stretchX_start = (int)(tex.getWidth()*0.35f);
//stretchX_end = (int)(tex.getWidth()*0.65f);

	// LEFT TOP CELL
	if (stretchX_start > l) {
		mCell[CELL_LT].mIsValid = true;
		mCell[CELL_LT].mSrc = ci::Area(l, t, stretchX_start-1, stretchY_start-1);
	}
	// RIGHT TOP CELL
	if (stretchX_end < tex.getWidth()) {
		mCell[CELL_RT].mIsValid = true;
		mCell[CELL_RT].mSrc = ci::Area(stretchX_end+1, t, r, stretchY_start-1);
	}
	// MIDDLE TOP CELL
	mCell[CELL_MT].mIsValid = true;
	mCell[CELL_MT].mSrc = ci::Area(stretchX_start, t, stretchX_end, stretchY_start-1);
}

void NinePatch::Patch::buildDestinations(const float width, const float height)
{
	// LEFT TOP CELL
	if (mCell[CELL_LT].mIsValid) {
		const ci::Vec2f	size = mCell[CELL_LT].size();
		mCell[CELL_LT].mDst = ci::Rectf(0.0f, 0.0f, size.x, size.y);
	}
	// RIGHT TOP CELL
	if (mCell[CELL_RT].mIsValid) {
		const ci::Vec2f	size = mCell[CELL_RT].size();
		mCell[CELL_RT].mDst = ci::Rectf(width - size.x, 0.0f, width, size.y);
	}
	// MIDDLE TOP CELL
	if (mCell[CELL_MT].mIsValid) {
		const ci::Vec2f	size = mCell[CELL_MT].size();
		mCell[CELL_MT].mDst = ci::Rectf(mCell[CELL_LT].size().x, 0.0f, width - mCell[CELL_RT].size().x, size.y);
	}
}

void NinePatch::Patch::draw(const ci::gl::Texture& tex)
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
