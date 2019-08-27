#include "stdafx.h"

#include <ds/ui/sprite/util/clip_plane.h>

#include "line.h"

namespace ds {
namespace ui {
namespace {
char BLOB_TYPE = 0;

const DirtyState& POINTS_DIRTY		= INTERNAL_A_DIRTY;
const DirtyState& MITER_LIMIT_DIRTY = INTERNAL_B_DIRTY;
const DirtyState& LINE_WIDTH_DIRTY  = INTERNAL_C_DIRTY;
const DirtyState& SMOOTH_DIRTY		= INTERNAL_D_DIRTY;
const DirtyState& START_STOP_DIRTY  = INTERNAL_E_DIRTY;

const char POINTS_ATT	 = 80;
const char MITER_ATT	  = 81;
const char LINE_WIDTH_ATT = 82;
const char SMOOTH_ATT	 = 83;
const char START_STOP_ATT = 84;

const ds::BitMask SPRITE_LOG = ds::Logger::newModule("Line Sprite");

const std::string lineFrag =
		"#version 150\n"
		"in VertexData{\n"
		"    vec2 mTexCoord;\n"
		"    vec4 mColor;\n"
		"} VertexIn;\n"
		"uniform sampler2D  tex0;\n"
		"uniform bool       useTexture;\n"
		"uniform bool       preMultiply;\n"
		"uniform float      lineStart;\n"
		"uniform float      lineEnd;\n"
		"out vec4 oColor;\n"
		"void main()\n"
		"{\n"
		"    oColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"    oColor *= VertexIn.mColor;\n"
		"    if(VertexIn.mTexCoord.x <= lineStart || VertexIn.mTexCoord.x > lineEnd) discard;"
		"}";

const std::string lineVert =
		"#version 150\n"
		"uniform mat4 ciModelViewProjection;\n"
		"uniform mat4		ciModelView;\n"
		"uniform vec4		uClipPlane0;\n"
		"uniform vec4 		uClipPlane1;\n"
		"uniform vec4		uClipPlane2;\n"
		"uniform vec4		uClipPlane3;\n"
		"in vec4 ciPosition;\n"
		"in vec4 ciColor;\n"
		"out VertexData{\n"
		"    vec4 ciPosition;\n"
		"    vec4 ciColor;\n"
		"} VertexOut;\n"
		"void main(void)\n"
		"{\n"
		"   VertexOut.ciPosition = ciPosition;\n"
		"   VertexOut.ciColor = ciColor;\n"
		"	gl_Position = ciModelViewProjection * ciPosition;\n"
		"}";

const std::string lineGeom =
		"#version 150\n"
		"uniform float THICKNESS;    // the thickness of the line in pixels\n"
		"uniform float MITER_LIMIT;  // 1.0: always miter, -1.0: never miter, 0.75: default\n"
		"uniform mat4 ciModelView;\n"
		"uniform mat4 ciModelMatrix;\n"
		"uniform mat4 ciModelViewProjection;\n"
		"uniform vec4 uClipPlane0;\n"
		"uniform vec4 uClipPlane1;\n"
		"uniform vec4 uClipPlane2;\n"
		"uniform vec4 uClipPlane3;\n"
		"layout(lines_adjacency) in;\n"
		"layout(triangle_strip, max_vertices = 7) out;\n"
		"vec2 p[4];\n"
		"in VertexData {\n"
		"    vec4 ciPosition;\n"
		"    vec4 ciColor;\n"
		"} VertexIn[4];\n"
		"out VertexData {\n"
		"    vec2 mTexCoord;\n"
		"    vec4 mColor;\n"
		"} VertexOut;\n"
		"void outputClip(vec4 vertex, int idx) {\n"
		"    gl_Position      = ciModelViewProjection * vec4((p[idx] + vertex.xy), 0.0, 1.0);\n"
		"    VertexOut.mColor = VertexIn[idx].ciColor;\n"
		"    VertexOut.mTexCoord = vec2(VertexIn[idx].ciPosition.w);\n"
		"    vertex           = ciModelView * vec4((p[idx] + vertex.xy), 0.0, 1.0);\n"
		"    gl_ClipDistance[0] = dot(vertex, uClipPlane0);\n"
		"    gl_ClipDistance[1] = dot(vertex, uClipPlane1);\n"
		"    gl_ClipDistance[2] = dot(vertex, uClipPlane2);\n"
		"    gl_ClipDistance[3] = dot(vertex, uClipPlane3);\n"
		"}\n"
		"void main(void) {\n"
		"    // get the four vertices passed to the shader:\n"
		"    p[0] = VertexIn[0].ciPosition.xy;  // start of previous segment\n"
		"    p[1] = VertexIn[1].ciPosition.xy;  // end of previous segment, start of current segment\n"
		"    p[2] = VertexIn[2].ciPosition.xy;  // end of current segment, start of next segment\n"
		"    p[3] = VertexIn[3].ciPosition.xy;  // end of next segment\n"
		"    // determine the direction of each of the 3 segments (previous, current, next)\n"
		"    vec2 v0 = normalize(p[1] - p[0]);\n"
		"    vec2 v1 = normalize(p[2] - p[1]);\n"
		"    vec2 v2 = normalize(p[3] - p[2]);\n"
		"    // determine the normal of each of the 3 segments (previous, current, next)\n"
		"    vec2 n0 = vec2(-v0.y, v0.x);\n"
		"    vec2 n1 = vec2(-v1.y, v1.x);\n"
		"    vec2 n2 = vec2(-v2.y, v2.x);\n"
		"    // determine miter lines by averaging the normals of the 2 segments\n"
		"    vec2 miter_a = normalize(n0 + n1);  // miter at start of current segment\n"
		"    vec2 miter_b = normalize(n1 + n2);  // miter at end of current segment\n"
		"    // determine the length of the miter by projecting it onto normal and then inverse it\n"
		"    float length_a = THICKNESS / dot(miter_a, n1);\n"
		"    float length_b = THICKNESS / dot(miter_b, n1);\n"
		"    // prevent excessively long miters at sharp corners\n"
		"    if (dot(v0, v1) < -MITER_LIMIT) {\n"
		"        miter_a  = n1;\n"
		"        length_a = THICKNESS;\n"
		"        // close the gap\n"
		"        if (dot(v0, n1) > 0) {\n"
		"            outputClip(vec4((THICKNESS * n0), 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            outputClip(vec4((THICKNESS * n1), 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            outputClip(vec4(0.0, 0.0, 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            EndPrimitive();\n"
		"        } else {\n"
		"            outputClip(vec4((-THICKNESS * n1), 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            outputClip(vec4((-THICKNESS * n0), 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            outputClip(vec4(0.0, 0.0, 0.0, 1.0), 1);\n"
		"            EmitVertex();\n"
		"            EndPrimitive();\n"
		"        }\n"
		"    }\n"
		"    if (dot(v1, v2) < -MITER_LIMIT) {\n"
		"        miter_b  = n1;\n"
		"        length_b = THICKNESS;\n"
		"    }\n"
		"    // generate the triangle strip\n"
		"    outputClip(vec4((length_a * miter_a), 0.0, 1.0), 1);\n"
		"    EmitVertex();\n"
		"    outputClip(vec4((-length_a * miter_a), 0.0, 1.0), 1);\n"
		"    EmitVertex();\n"
		"    outputClip(vec4((length_b * miter_b), 0.0, 1.0), 2);\n"
		"    EmitVertex();\n"
		"    outputClip(vec4((-length_b * miter_b), 0.0, 1.0), 2);\n"
		"    EmitVertex();\n"
		"    EndPrimitive();\n"
		"}\n";
}

void LineSprite::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
}

void LineSprite::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<LineSprite>(r); });
}

LineSprite::LineSprite(ds::ui::SpriteEngine& eng, const std::vector<ci::vec2>& points)
  : ds::ui::Sprite(eng)
  , mLineStart(0.0f)
  , mLineEnd(1.0f)
  , mLineWidth(1.0f)
  , mMiterLimit(0.5f)
  , mSmoothSpline(false) {
	// Load shaders
	try {
		mShader = ci::gl::GlslProg::create(lineVert, lineFrag, lineGeom);
	} catch (const std::exception& e) {
		DS_LOG_ERROR("Could not compile shader for LineSprite! Error: " << e.what());
		return;
	}

	mBlobType = BLOB_TYPE;
	setTransparent(false);

	setPoints(points);
	mLayoutFixedAspect = true;
}

void LineSprite::addPoint(const ci::vec2 point) {
	mPoints.push_back(point);
	mNeedsBatchUpdate = true;
	markAsDirty(POINTS_DIRTY);
}

void LineSprite::setPoints(const std::vector<ci::vec2>& points) {
	mPoints			  = points;
	mNeedsBatchUpdate = true;
	markAsDirty(POINTS_DIRTY);
}

void LineSprite::clearPoints() {
	mPoints.clear();
	mNeedsBatchUpdate = true;
	markAsDirty(POINTS_DIRTY);
}

void LineSprite::setStartPercentage(float startAtPercent) { setStartEndPercentages(startAtPercent, mLineEnd); }

void LineSprite::setEndPercentage(float endAtPercent) { setStartEndPercentages(mLineStart, endAtPercent); }

void LineSprite::setStartEndPercentages(float startAtPercent, float endAtPercent) {
	mLineStart = startAtPercent;
	mLineEnd   = endAtPercent;
	markAsDirty(START_STOP_DIRTY);
}

void LineSprite::setLineWidth(const float linewidth) {
	mLineWidth		  = linewidth;
	mNeedsBatchUpdate = true;
	markAsDirty(LINE_WIDTH_DIRTY);
}

void LineSprite::setSmoothing(const bool doSmooth) {
	mSmoothSpline	 = doSmooth;
	mNeedsBatchUpdate = true;
	markAsDirty(SMOOTH_DIRTY);
}


void LineSprite::setMiterLimit(const float miterLimit) {
	mMiterLimit		  = miterLimit;
	mNeedsBatchUpdate = true;
	markAsDirty(MITER_LIMIT_DIRTY);
}

void LineSprite::buildVbo() {
	auto localPoints = mPoints;
	if (mSmoothSpline) {
		auto spline = ci::BSpline2f(mPoints, 2, false, true);

		localPoints.clear();
		const auto slices = spline.getLength(0.0f, 1.0f) / 4.0f;
		for (int i = 0; i <= int(slices); ++i) {
			localPoints.push_back(spline.getPosition((float)i / slices));
		}
	}

	std::vector<ci::vec4> vertices;
	// to improve performance, make room for the vertices + 2 adjacency vertices
	vertices.reserve(localPoints.size() + 2);

	// first, add an adjacency vertex at the beginning
	vertices.push_back(ci::vec4(2.0f * ci::vec3(localPoints[0], 0) - ci::vec3(localPoints[1], 0), 0));
	// find the total length of the line for vec4 calculations
	float totalLength = 0.f;
	for (size_t i = 1; i < localPoints.size(); i++) {
		totalLength += glm::distance(localPoints[i-1], localPoints[i]);
	}
	// next, add all 2D points as 3D vertices
	float currentLength = 0.f;
	ci::vec2 prevP;
	for (size_t i = 0; i < localPoints.size(); i++) {
		const ci::vec2& p = localPoints[i];
		if (i != 0) currentLength += glm::distance(p, prevP);
		prevP = p;
		vertices.push_back(ci::vec4(p, 0, currentLength / totalLength));
	}

	// next, add an adjacency vertex at the end
	size_t n = localPoints.size();
	vertices.push_back(ci::vec4(2.0f * ci::vec3(localPoints[n - 1], 0) - ci::vec3(localPoints[n - 2], 0), 1));

	// now that we have a list of vertices, create the index buffer
	n = vertices.size() - 2;
	std::vector<uint16_t> indices;
	indices.reserve(n * 4);

	for (uint16_t i = 1; i < static_cast<uint16_t>(vertices.size()) - 2; ++i) {
		indices.push_back(i - 1);
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
	}

	// finally, create the mesh
	ci::gl::VboMesh::Layout layout;
	layout.attrib(ci::geom::POSITION, 4);

	mVboMesh = ci::gl::VboMesh::create(static_cast<uint32_t>(vertices.size()), GL_LINES_ADJACENCY_EXT, {layout}, static_cast<uint32_t>(indices.size()));
	mVboMesh->bufferAttrib(ci::geom::POSITION, vertices.size() * sizeof(ci::vec4), vertices.data());
	mVboMesh->bufferIndices(indices.size() * sizeof(uint16_t), indices.data());
}

void LineSprite::buildRenderBatch() {
	if (!mNeedsBatchUpdate) return;
	mNeedsBatchUpdate = false;

	if ((mSmoothSpline && mPoints.size() > 2) || (!mSmoothSpline && mPoints.size() > 1)) {
		buildVbo();
		if (mShader && mVboMesh) {
			mShader->uniform("MITER_LIMIT", mMiterLimit);
			mShader->uniform("THICKNESS", mLineWidth);
			clip_plane::passClipPlanesToShader(mShader);

			mRenderBatch = ci::gl::Batch::create(mVboMesh, mShader);
		}
	} else {
		mRenderBatch.reset();
	}
}

void LineSprite::onBuildRenderBatch() {}

bool LineSprite::contains(const ci::vec3& point, const float pad) const {
	// If I don't check this, then sprites with no size are always picked.
	// Someone who knows the math can probably address the root issue.
	// if(mWidth < 0.001f || mHeight < 0.001f) return false;
	// Same deal as above.
	// if (mScale.x <= 0.0f || mScale.y <= 0.0f) return false;
	// May have negative scaling
	if (mScale.x == 0.0f || mScale.y == 0.0f) return false;

	buildGlobalTransform();

	for (int i = 0; i < mPoints.size() - 1; ++i) {
		auto testPt = mInverseGlobalTransform * ci::vec4(point, 1.0f);
		auto bounds = ci::Rectf(mPoints[i], mPoints[i + 1]);
		bounds.canonicalize();
		bounds.inflate(ci::vec2(mLineWidth));
		if (bounds.contains(ci::vec2(testPt))) {
			return true;
		}
	}
	return false;
}

bool LineSprite::getInnerHit(const ci::vec3& pos) const {
	// Hit testing for the actual line
	auto distCalc = [](ci::vec2 A, ci::vec2 B, ci::vec2 P) {
		double normalLength = std::sqrt((B.x - A.x) * (B.x - A.x) + (B.y - A.y) * (B.y - A.y));
		return std::abs((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x)) / normalLength;
	};

	for (int i = 0; i < mPoints.size() - 1; ++i) {
		auto testPt = mInverseGlobalTransform * ci::vec4(pos, 1.0f);
		auto bounds = ci::Rectf(mPoints[i], mPoints[i + 1]);
		bounds.canonicalize();
		bounds.inflate(ci::vec2(mLineWidth));
		if (bounds.contains(ci::vec2(testPt)) && distCalc(mPoints[i], mPoints[i + 1], ci::vec2(testPt)) <= mLineWidth) {
			return true;
		}
	}
	return false;
}

void LineSprite::drawLocalClient() {
	if (mRenderBatch && ((mSmoothSpline && mPoints.size() > 2) || (!mSmoothSpline && mPoints.size() > 1))) {
		clip_plane::passClipPlanesToShader(mShader);
		mShader->uniform("lineStart", mLineStart);
		mShader->uniform("lineEnd", mLineEnd);
		mRenderBatch->draw();
	}
}

void LineSprite::writeAttributesTo(ds::DataBuffer& buf) {
	Sprite::writeAttributesTo(buf);

	if (mDirty.has(POINTS_DIRTY)) {
		buf.add(POINTS_ATT);
		buf.add((size_t)mPoints.size());
		for (const auto& p : mPoints) {
			buf.add(p);
		}
	}
	if (mDirty.has(MITER_LIMIT_DIRTY)) {
		buf.add(MITER_ATT);
		buf.add(mMiterLimit);
	}
	if (mDirty.has(LINE_WIDTH_DIRTY)) {
		buf.add(LINE_WIDTH_ATT);
		buf.add(mLineWidth);
	}
	if (mDirty.has(SMOOTH_DIRTY)) {
		buf.add(SMOOTH_ATT);
		buf.add(mSmoothSpline);
	}
	if (mDirty.has(START_STOP_DIRTY)) {
		buf.add(START_STOP_ATT);
		buf.add(mLineStart);
		buf.add(mLineEnd);
	}
}

void LineSprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == POINTS_ATT) {
		size_t numPoints = buf.read<size_t>();
		for (int i = 0; i < numPoints; ++i) {
			mPoints.push_back(buf.read<ci::vec2>());
		}
		mNeedsBatchUpdate = true;
	} else if (attributeId == MITER_ATT) {
		mMiterLimit		  = buf.read<float>();
		mNeedsBatchUpdate = true;

	} else if (attributeId == SMOOTH_ATT) {
		mSmoothSpline	 = buf.read<bool>();
		mNeedsBatchUpdate = true;
	} else if (attributeId == LINE_WIDTH_ATT) {
		mLineWidth		  = buf.read<float>();
		mNeedsBatchUpdate = true;
	} else if (attributeId == START_STOP_ATT) {
		mLineStart = buf.read<float>();
		mLineEnd   = buf.read<float>();
	} else {
		Sprite::readAttributeFrom(attributeId, buf);
	}
}

}  // namespace experiments
}  // namespace experiments
