#include "ds/ui/sprite/circle_border.h"

#include <ds/app/environment.h>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

using namespace ci;

namespace ds {
namespace ui {

namespace {
	char				BLOB_TYPE = 0;

	const DirtyState&	BORDER_WIDTH_DIRTY = INTERNAL_A_DIRTY;
	
	const char			BORDER_WIDTH_ATT = 80;
	
	const ds::BitMask   SPRITE_LOG = ds::Logger::newModule("circle border sprite");
}

void CircleBorder::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void CircleBorder::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<CircleBorder>(r); });
}

CircleBorder::CircleBorder(SpriteEngine& engine)
	: inherited(engine)
	, mBorderWidth(0.0f)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	mSpriteShader.setShaders(Environment::getAppFolder("data/shaders"), "circle_border");
	mLayoutFixedAspect = true;
}

CircleBorder::CircleBorder(SpriteEngine& engine, const float width)
	: inherited(engine)
	, mBorderWidth(width)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	markAsDirty(BORDER_WIDTH_DIRTY);
	mSpriteShader.setShaders(Environment::getAppFolder("data/shaders"), "circle_border");
	mLayoutFixedAspect = true;
	updateShaderExtraData();
}

void CircleBorder::setBorderWidth(const float borderWidth) {
	mBorderWidth = borderWidth;
	updateShaderExtraData();
	markAsDirty(BORDER_WIDTH_DIRTY);
}

void CircleBorder::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if(mDirty.has(BORDER_WIDTH_DIRTY)) {
		buf.add(BORDER_WIDTH_ATT);
		buf.add(mBorderWidth);
	}
}

void CircleBorder::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == BORDER_WIDTH_ATT) {
		mBorderWidth = buf.read<float>();
		updateShaderExtraData();
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void CircleBorder::updateShaderExtraData() {
	mShaderExtraData.x = mBorderWidth;
}

} // namespace ui
} // namespace ds
