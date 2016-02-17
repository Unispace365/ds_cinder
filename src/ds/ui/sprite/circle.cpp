#include "circle.h"

#include <map>
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

	const DirtyState&	RADIUS_DIRTY = INTERNAL_A_DIRTY;
	const DirtyState&	FILLED_DIRTY = INTERNAL_B_DIRTY;
	const DirtyState&	LINE_WIDTH_DIRTY = INTERNAL_C_DIRTY;

	const char			RADIUS_ATT = 80;
	const char			FILLED_ATT = 81;
	const char			LINE_WIDTH_ATT = 82;

	const ds::BitMask   SPRITE_LOG = ds::Logger::newModule("circle sprite");
}

void Circle::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void Circle::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Circle>(r); });
}

Circle::Circle(SpriteEngine& engine)
	: inherited(engine)
	, mFilled(true)
	, mRadius(0.0f)
	, mVertices(nullptr)
	, mIgnoreSizeUpdates(false)
	, mLineWidth(1.0f)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
}

Circle::Circle(SpriteEngine& engine, const bool filled, const float radius)
	: inherited(engine)
	, mFilled(filled)
	, mRadius(radius)
	, mVertices(nullptr)
	, mIgnoreSizeUpdates(false)
	, mLineWidth(1.0f)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setRadius(mRadius);
	setFilled(mFilled);
}

Circle::~Circle(){
	if(mVertices){
		delete [] mVertices;
		mVertices = nullptr;
	}
}

void Circle::updateServer(const UpdateParams& up) {
	inherited::updateServer(up);
}

void Circle::drawLocalClient() {
	if(!mVertices) return;

	ci::gl::lineWidth(mLineWidth);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, mVertices);
	if(mFilled){
		glDrawArrays(GL_TRIANGLE_FAN, 0, mNumberOfSegments + 2);
	} else {
		glDrawArrays(GL_LINE_LOOP, 0, mNumberOfSegments);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Circle::drawLocalServer() {
	drawLocalClient();
}

void Circle::writeAttributesTo(ds::DataBuffer& buf) {
	inherited::writeAttributesTo(buf);

	if(mDirty.has(RADIUS_DIRTY)) {
		buf.add(RADIUS_ATT);
		buf.add(mRadius);
	}
	if (mDirty.has(FILLED_DIRTY)) {
		buf.add(FILLED_ATT);
		buf.add(mFilled);
	}
	if(mDirty.has(LINE_WIDTH_DIRTY)){
		buf.add(LINE_WIDTH_ATT);
		buf.add(mLineWidth);
	}
}

void Circle::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == RADIUS_ATT) {
		mRadius = buf.read<float>();
		init();

	} else if(attributeId == FILLED_ATT) {
		mFilled = buf.read<bool>();
		init();

	} else if(attributeId == LINE_WIDTH_ATT) {
		mLineWidth = buf.read<float>();

	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void Circle::onSizeChanged() {
	if(!mIgnoreSizeUpdates) {
		float minDiameter = mWidth;
		if(minDiameter > mHeight) {
			minDiameter = mHeight;
		}
		setRadius(minDiameter * 0.5f);
	}
}

void Circle::init() {
	if(mVertices){
		delete [] mVertices;
		mVertices = nullptr;
	}
	mNumberOfSegments = 0;

	if(mRadius <= 0.0f) return;

	mIgnoreSizeUpdates = true;
	setSize(mRadius * 2.0f, mRadius * 2.0f);
	mIgnoreSizeUpdates = false;

	// GN: From ci::gl::drawSolidCircle()
	// automatically determine the number of segments from the circumference
	mNumberOfSegments = (int)math<double>::floor(mRadius * M_PI * 2);
	
	if(mNumberOfSegments < 2) mNumberOfSegments = 2;
	ci::Vec2f center = ci::Vec2f(mRadius, mRadius);
	if(mFilled){
		mVertices = new float[(mNumberOfSegments + 2) * 2];
		mVertices[0] = center.x;
		mVertices[1] = center.y;
		for(int s = 0; s <= mNumberOfSegments; s++) {
			float t = s / (float)mNumberOfSegments * 2.0f * 3.14159f;
			mVertices[(s + 1) * 2 + 0] = center.x + math<float>::cos(t) * mRadius;
			mVertices[(s + 1) * 2 + 1] = center.y + math<float>::sin(t) * mRadius;
		}
	} else {
		mVertices = new float[mNumberOfSegments * 2];
		for(int s = 0; s < mNumberOfSegments; s++) {
			float t = s / (float)mNumberOfSegments * 2.0f * 3.14159f;
			mVertices[s * 2 + 0] = center.x + math<float>::cos(t) * mRadius;
			mVertices[s * 2 + 1] = center.y + math<float>::sin(t) * mRadius;
		}
	}
}

void Circle::setFilled(const bool filled){
	mFilled = filled;
	init();
	markAsDirty(FILLED_DIRTY);
}

void Circle::setRadius(const float radius){
	mRadius = radius;
	init();
	markAsDirty(RADIUS_DIRTY);
}

void Circle::setLineWidth(const float lineWidth){
	mLineWidth = lineWidth;
	markAsDirty(LINE_WIDTH_DIRTY);
}

} // namespace ui
} // namespace ds
