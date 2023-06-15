#include "stdafx.h"

#include "circle.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/app/environment.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include <map>

using namespace ci;

namespace ds { namespace ui {

	namespace {

		char BLOB_TYPE = 0;

		const DirtyState& RADIUS_DIRTY		 = INTERNAL_A_DIRTY;
		const DirtyState& FILLED_DIRTY		 = INTERNAL_B_DIRTY;
		const DirtyState& LINE_WIDTH_DIRTY	 = INTERNAL_C_DIRTY;
		const DirtyState& NUM_SEGMENTS_DIRTY = INTERNAL_D_DIRTY;

		const char RADIUS_ATT	  = 80;
		const char FILLED_ATT	  = 81;
		const char LINE_WIDTH_ATT = 82;
		const char NUM_SEGS_ATT	  = 83;

		const ds::BitMask SPRITE_LOG = ds::Logger::newModule("circle sprite");
	} // namespace


	///////////////////////////////////////////////////////////////////////////////////////
	// Circle
	DsCircleGeom::DsCircleGeom()
	  : mRequestedSubdivisions(-1)
	  , mCenter(0, 0)
	  , mRadius(1.0f) {
		updateVertexCounts();
	}

	DsCircleGeom& DsCircleGeom::subdivisions(int subdivs) {
		mRequestedSubdivisions = subdivs;
		updateVertexCounts();
		return *this;
	}

	DsCircleGeom& DsCircleGeom::radius(float radius) {
		mRadius = radius;
		updateVertexCounts();
		return *this;
	}

	// If numSegments<0, calculate based on radius
	void DsCircleGeom::updateVertexCounts() {
		if (mRequestedSubdivisions <= 0)
			mNumSubdivisions = (int)math<double>::floor(mRadius * float(M_PI * 2));
		else
			mNumSubdivisions = mRequestedSubdivisions;

		if (mNumSubdivisions < 3) mNumSubdivisions = 3;
		mNumVertices = mNumSubdivisions + 1 + 1;
	}

	size_t DsCircleGeom::getNumVertices() const {
		return mNumVertices;
	}

	uint8_t DsCircleGeom::getAttribDims(ci::geom::Attrib attr) const {
		switch (attr) {
		case ci::geom::Attrib::POSITION:
			return 2;
		case ci::geom::Attrib::NORMAL:
			return 3;
		case ci::geom::Attrib::TEX_COORD_0:
			return 2;
		default:
			return 0;
		}
	}

	ci::geom::AttribSet DsCircleGeom::getAvailableAttribs() const {
		return {ci::geom::Attrib::POSITION, ci::geom::Attrib::NORMAL, ci::geom::Attrib::TEX_COORD_0};
	}

	void DsCircleGeom::loadInto(ci::geom::Target* target, const ci::geom::AttribSet& /*requestedAttribs*/) const {
		std::vector<vec2> positions, texCoords;
		std::vector<vec3> normals;

		positions.reserve(mNumVertices);
		texCoords.reserve(mNumVertices);
		normals.reserve(mNumVertices);

		// center
		positions.emplace_back(mCenter);
		texCoords.emplace_back(0.5f, 0.5f);
		normals.emplace_back(0, 0, 1);

		// iterate the segments
		const float tDelta = 1 / (float)(mNumSubdivisions - 1) * 2.0f * 3.14159f;
		float		t	   = 0;
		for (int s = 0; s <= (mNumSubdivisions - 1); s++) {
			vec2 unit(math<float>::cos(t), math<float>::sin(t));
			positions.emplace_back(mCenter + unit * mRadius);
			texCoords.emplace_back(unit * 0.5f + vec2(0.5f));
			normals.emplace_back(0, 0, 1);
			t += tDelta;
		}

		vec2 unit(math<float>::cos(0), math<float>::sin(0));
		positions.emplace_back(mCenter + unit * mRadius);
		texCoords.emplace_back(unit * 0.5f + vec2(0.5f));
		normals.emplace_back(0, 0, 1);

		target->copyAttrib(ci::geom::Attrib::POSITION, 2, 0, (const float*)positions.data(), mNumVertices);
		target->copyAttrib(ci::geom::Attrib::NORMAL, 3, 0, (const float*)normals.data(), mNumVertices);
		target->copyAttrib(ci::geom::Attrib::TEX_COORD_0, 2, 0, (const float*)texCoords.data(), mNumVertices);
	}


	void Circle::installAsServer(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
	}

	void Circle::installAsClient(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<Circle>(r); });
	}

	Circle::Circle(SpriteEngine& engine)
	  : inherited(engine)
	  , mFilled(true)
	  , mRadius(0.0f)
	  , mIgnoreSizeUpdates(false)
	  , mLineWidth(1.0f)
	  , mNumberOfSegments(0) {
		mBlobType = BLOB_TYPE;
		setTransparent(false);
		mLayoutFixedAspect = true;
	}

	Circle::Circle(SpriteEngine& engine, const bool filled, const float radius)
	  : inherited(engine)
	  , mFilled(!filled)
	  , mRadius(radius)
	  , mIgnoreSizeUpdates(false)
	  , mLineWidth(1.0f)
	  , mNumberOfSegments(0) {
		mBlobType = BLOB_TYPE;
		setTransparent(false);

		setRadius(mRadius);
		setFilled(filled);
		mLayoutFixedAspect = true;
	}

	void Circle::drawLocalClient() {

		if (mRadius <= 0.0f) return;

		if (mRenderBatch) {
			mRenderBatch->draw();
		} else {
			if (mFilled) {
				ci::gl::drawSolidCircle(ci::vec2(mRadius, mRadius), mRadius);
			} else {
				ci::gl::lineWidth(mLineWidth);
				ci::gl::drawStrokedCircle(ci::vec2(mRadius, mRadius), mRadius);
			}
		}
	}

	void Circle::drawLocalServer() {
		drawLocalClient();
	}

	void Circle::writeAttributesTo(ds::DataBuffer& buf) {
		inherited::writeAttributesTo(buf);

		if (mDirty.has(RADIUS_DIRTY)) {
			buf.add(RADIUS_ATT);
			buf.add(mRadius);
		}
		if (mDirty.has(FILLED_DIRTY)) {
			buf.add(FILLED_ATT);
			buf.add(mFilled);
		}
		if (mDirty.has(LINE_WIDTH_DIRTY)) {
			buf.add(LINE_WIDTH_ATT);
			buf.add(mLineWidth);
		}
		if (mDirty.has(NUM_SEGMENTS_DIRTY)) {
			buf.add(NUM_SEGS_ATT);
			buf.add(mNumberOfSegments);
		}
	}

	void Circle::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
		if (attributeId == RADIUS_ATT) {
			mRadius			  = buf.read<float>();
			mNeedsBatchUpdate = true;

		} else if (attributeId == FILLED_ATT) {
			mFilled			  = buf.read<bool>();
			mNeedsBatchUpdate = true;

		} else if (attributeId == LINE_WIDTH_ATT) {
			mLineWidth		  = buf.read<float>();
			mNeedsBatchUpdate = true;

		} else if (attributeId == NUM_SEGS_ATT) {
			mNumberOfSegments = buf.read<int>();
			mNeedsBatchUpdate = true;

		} else {
			inherited::readAttributeFrom(attributeId, buf);
		}
	}

	void Circle::onSizeChanged() {
		if (!mIgnoreSizeUpdates) {
			float minDiameter = mWidth;
			if (minDiameter > mHeight) {
				minDiameter = mHeight;
			}
			setRadius(minDiameter * 0.5f);
		}
	}

	void Circle::onBuildRenderBatch() {
		if (mRadius <= 0.0f) return;

		if (getWidth() != mRadius * 2.0f || getHeight() != mRadius * 2.0f) {
			mIgnoreSizeUpdates = true;
			setSize(mRadius * 2.0f, mRadius * 2.0f);
			mIgnoreSizeUpdates = false;
		}

		if (mFilled) {
			auto theCircle = DsCircleGeom().radius(mRadius+mLineWidth).center(ci::vec2(mRadius, mRadius));
			if (mNumberOfSegments > 1) {
				theCircle.subdivisions(mNumberOfSegments);
			}
			if (mRenderBatch)
				mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theCircle));
			else
				mRenderBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
		} else {
			auto theCircle = ci::geom::Ring().radius(mRadius).width(mLineWidth).center(ci::vec2(mRadius, mRadius));
			if (mNumberOfSegments > 1) {
				theCircle.subdivisions(mNumberOfSegments);
			}
			if (mRenderBatch)
				mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theCircle));
			else
				mRenderBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
		}
	}

	void Circle::setFilled(const bool filled) {
		if (filled == mFilled) return;
		mFilled			  = filled;
		mNeedsBatchUpdate = true;
		markAsDirty(FILLED_DIRTY);
	}

	void Circle::setRadius(const float radius) {
		if (mRadius == radius) return;
		mRadius = radius;

		mIgnoreSizeUpdates = true;
		setSize(mRadius * 2.0f, mRadius * 2.0f);
		mIgnoreSizeUpdates = false;

		mNeedsBatchUpdate = true;
		markAsDirty(RADIUS_DIRTY);
	}

	void Circle::setLineWidth(const float lineWidth) {
		if (mLineWidth == lineWidth) return;
		mLineWidth		  = lineWidth;
		mNeedsBatchUpdate = true;
		markAsDirty(LINE_WIDTH_DIRTY);
	}

	void Circle::setNumberOfSegments(const int numSegments) {
		if (mNumberOfSegments == numSegments) return;
		mNumberOfSegments = numSegments;
		mNeedsBatchUpdate = true;
		markAsDirty(NUM_SEGMENTS_DIRTY);
	}

}} // namespace ds::ui
