#include "stdafx.h"

#include "circle.h"

#include <map>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/app/environment.h"

#include <gl/GL.h>

using namespace ci;

namespace ds {
namespace ui {

namespace {

	char				BLOB_TYPE = 0;

	const DirtyState&	RADIUS_DIRTY = INTERNAL_A_DIRTY;
	const DirtyState&	FILLED_DIRTY = INTERNAL_B_DIRTY;
	const DirtyState&	LINE_WIDTH_DIRTY = INTERNAL_C_DIRTY;
	const DirtyState&	NUM_SEGMENTS_DIRTY = INTERNAL_D_DIRTY;

	const char			RADIUS_ATT = 80;
	const char			FILLED_ATT = 81;
	const char			LINE_WIDTH_ATT = 82;
	const char			NUM_SEGS_ATT = 83;

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
	, mIgnoreSizeUpdates(false)
	, mLineWidth(1.0f)
	, mNumberOfSegments(0)
	, mNeedsInit(true)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	mLayoutFixedAspect = true;
}

Circle::Circle(SpriteEngine& engine, const bool filled, const float radius)
	: inherited(engine)
	, mFilled(filled)
	, mRadius(radius)
	, mIgnoreSizeUpdates(false)
	, mLineWidth(1.0f)
	, mNumberOfSegments(0)
	, mNeedsInit(true)
{
	mBlobType = BLOB_TYPE;
	setTransparent(false);

	setRadius(mRadius);
	setFilled(mFilled);
	mLayoutFixedAspect = true;
}

Circle::~Circle(){
}


void Circle::drawLocalClient() {


	if(mNeedsInit){
		init();
	}

#ifdef USE_BATCH_DRAWING


	if(mCircleBatch){
		mCircleBatch->draw();
	}

#else 

	if(mFilled){
		ci::gl::drawSolidCircle(ci::vec2(mRadius, mRadius), mRadius);
	} else {
		ci::gl::lineWidth(mLineWidth);
		ci::gl::drawStrokedCircle(ci::vec2(mRadius, mRadius), mRadius);
	}


#endif

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
	if(mDirty.has(FILLED_DIRTY)) {
		buf.add(FILLED_ATT);
		buf.add(mFilled);
	}
	if(mDirty.has(LINE_WIDTH_DIRTY)){
		buf.add(LINE_WIDTH_ATT);
		buf.add(mLineWidth);
	}
	if(mDirty.has(NUM_SEGMENTS_DIRTY)){
		buf.add(NUM_SEGS_ATT);
		buf.add(mNumberOfSegments);
	}
}

void Circle::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if(attributeId == RADIUS_ATT) {
		mRadius = buf.read<float>();
		mNeedsInit = true;

	} else if(attributeId == FILLED_ATT) {
		mFilled = buf.read<bool>();
		mNeedsInit = true;

	} else if(attributeId == LINE_WIDTH_ATT) {
		mLineWidth = buf.read<float>();
		mNeedsInit = true;

	} else if(attributeId == NUM_SEGS_ATT) {
		mNumberOfSegments = buf.read<int>();
		mNeedsInit = true;

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
#ifdef USE_BATCH_DRAWING
	mCircleBatch = nullptr;
#endif

	mNeedsInit = false;

	if(mRadius <= 0.0f) return;

	mIgnoreSizeUpdates = true;
	setSize(mRadius * 2.0f, mRadius * 2.0f);
	mIgnoreSizeUpdates = false;

#ifdef USE_BATCH_DRAWING
	mSpriteShader.loadShaders();

	
	if(mFilled){
		auto theCircle = ci::geom::Circle().radius(mRadius).center(ci::vec2(mRadius, mRadius));
		if(mNumberOfSegments > 1){
			theCircle.subdivisions(mNumberOfSegments);
		}
		mCircleBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
	} else {
		auto theCircle = ci::geom::Ring().radius(mRadius).width(mLineWidth).center(ci::vec2(mRadius, mRadius));
		if(mNumberOfSegments > 1){
			theCircle.subdivisions(mNumberOfSegments);
		}
		mCircleBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
	}
#endif
}

void Circle::setFilled(const bool filled){
	mFilled = filled;
	mNeedsInit = true;
	markAsDirty(FILLED_DIRTY);
}

void Circle::setRadius(const float radius){
	mRadius = radius;

	mIgnoreSizeUpdates = true;
	setSize(mRadius * 2.0f, mRadius * 2.0f);
	mIgnoreSizeUpdates = false;

	mNeedsInit = true;
	markAsDirty(RADIUS_DIRTY);
}

void Circle::setLineWidth(const float lineWidth){
	mLineWidth = lineWidth;
	mNeedsInit = true;
	markAsDirty(LINE_WIDTH_DIRTY);
}

void Circle::setNumberOfSegments(const int numSegments){
	mNumberOfSegments = numSegments;
	mNeedsInit = true;
	markAsDirty(NUM_SEGMENTS_DIRTY);
}

} // namespace ui
} // namespace ds
