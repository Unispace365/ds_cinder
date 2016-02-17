#include "border.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

#include <gl/GL.h>

using namespace ci;

namespace ds {
namespace ui {

namespace {
	char				BLOB_TYPE = 0;

	const DirtyState&	BORDER_WIDTH_DIRTY = INTERNAL_A_DIRTY;
	
	const char			BORDER_WIDTH_ATT = 80;
	
	const ds::BitMask   SPRITE_LOG = ds::Logger::newModule("border sprite");
}

void Border::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void Border::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Border>(r); });
}

Border::Border(SpriteEngine& engine)
	: inherited(engine)
	, mBorderWidth(0.0f)
	, mVertices(nullptr)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
}

Border::Border(SpriteEngine& engine, const float width)
	: inherited(engine)
	, mBorderWidth(width)
	, mVertices(nullptr)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	rebuildVertices();
	markAsDirty(BORDER_WIDTH_DIRTY);
}

Border::~Border(){
	if(mVertices){
		delete [] mVertices;
		mVertices = nullptr;
	}
}

void Border::onSizeChanged() {
	inherited::onSizeChanged();
	rebuildVertices();
}

void Border::setBorderWidth(const float borderWidth) {
	mBorderWidth = borderWidth;
	rebuildVertices();
	markAsDirty(BORDER_WIDTH_DIRTY);
}

void Border::updateServer(const UpdateParams& up) {
	inherited::updateServer(up);
}

void Border::drawLocalClient() {
	if(!mVertices) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, mVertices);
	glDrawArrays(GL_QUADS, 0, 16);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Border::drawLocalServer() {
	drawLocalClient();
}

void Border::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if(mDirty.has(BORDER_WIDTH_DIRTY)) {
		buf.add(BORDER_WIDTH_ATT);
		buf.add(mBorderWidth);
	}
}

void Border::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == BORDER_WIDTH_ATT) {
		mBorderWidth = buf.read<float>();
		rebuildVertices();
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void Border::rebuildVertices() {
	if(mVertices){
		delete [] mVertices;
		mVertices = nullptr;
	}
	
	if(mBorderWidth <= 0.0f) return;

	const float w = getWidth();
	const float h = getHeight();

	mVertices = new float[32];

	int i = 0;
	// top quad
	mVertices[i++] = 0.0f;
	mVertices[i++] = h - mBorderWidth;
	mVertices[i++] = w;
	mVertices[i++] = h - mBorderWidth;
	mVertices[i++] = w;
	mVertices[i++] = h;
	mVertices[i++] = 0.0f;
	mVertices[i++] = h;

	// bottom quad
	mVertices[i++] = 0.0f;
	mVertices[i++] = 0.0f;
	mVertices[i++] = w;
	mVertices[i++] = 0.0f;
	mVertices[i++] = w;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = 0.0f;
	mVertices[i++] = mBorderWidth;

	// left quad
	mVertices[i++] = 0.0f;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = h - mBorderWidth;
	mVertices[i++] = 0.0f;
	mVertices[i++] = h - mBorderWidth;

	// right quad
	mVertices[i++] = w - mBorderWidth;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = w;
	mVertices[i++] = mBorderWidth;
	mVertices[i++] = w;
	mVertices[i++] = h - mBorderWidth;
	mVertices[i++] = w - mBorderWidth;
	mVertices[i++] = h - mBorderWidth;
}

} // namespace ui
} // namespace ds
