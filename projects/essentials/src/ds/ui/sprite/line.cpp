#include "stdafx.h"

#include "line.h"

namespace ds {
namespace ui {
namespace {
char BLOB_TYPE = 0;

const DirtyState& POINTS_DIRTY      = INTERNAL_A_DIRTY;
const DirtyState& MITER_LIMIT_DIRTY = INTERNAL_B_DIRTY;
const DirtyState& LINE_WIDTH_DIRTY  = INTERNAL_C_DIRTY;
const DirtyState& SMOOTH_DIRTY  = INTERNAL_D_DIRTY;

const char POINTS_ATT     = 80;
const char MITER_ATT      = 81;
const char LINE_WIDTH_ATT = 82;
const char SMOOTH_ATT = 83;

const ds::BitMask SPRITE_LOG = ds::Logger::newModule("Line Sprite");

const std::string lineFrag =
"#version 150\n"
"in VertexData{\n"
"    vec2 mTexCoord;\n"
"    vec3 mColor;\n"
"} VertexIn;\n"
"uniform sampler2D  tex0;\n"
"uniform bool       useTexture;\n"   // dummy, Engine always sends this anyway
"uniform bool       preMultiply;\n"  // dummy, Engine always sends this anyway
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"    oColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    //if (useTexture) {\n"
"        //oColor = texture2D( tex0, vec2(VertexIn.mTexCoord.x, 1.0-VertexIn.mTexCoord.y) );\n"
"    //}\n"
"    // Now do the normal colorize/optional premultiplication\n"
"    oColor.rgb *= VertexIn.mColor;\n"
"    //if (preMultiply)\n"
"        //oColor.rgb *= oColor.a;\n"
"}\n";

const std::string lineVert =
"#version 150\n"
"uniform mat4 ciModelViewProjection;\n"
"in vec4 ciPosition;\n"
"in vec3 ciColor;\n"
"out VertexData{\n"
"    vec3 mColor;\n"
"} VertexOut;\n"
"void main(void)\n"
"{\n"
"    VertexOut.mColor = ciColor;\n"
"    gl_Position = ciModelViewProjection * ciPosition;\n"
"}\n";

const std::string lineGeom =
"#version 150\n"
"uniform float    THICKNESS;        // the thickness of the line in pixels\n"
"uniform float    MITER_LIMIT;    // 1.0: always miter, -1.0: never miter, 0.75: default\n"
"uniform vec2    WIN_SCALE;        // the size of the viewport in pixels\n"
"layout( lines_adjacency ) in;\n"
"layout( triangle_strip, max_vertices = 7 ) out;\n"
"in VertexData{\n"
"    vec3 mColor;\n"
"} VertexIn[4];\n"
"out VertexData{\n"
"    vec2 mTexCoord;\n"
"    vec3 mColor;\n"
"} VertexOut;\n"
"vec2 toScreenSpace( vec4 vertex )\n"
"{\n"
"    return vec2( vertex.xy / vertex.w ) * WIN_SCALE;\n"
"}\n"
"void main( void ){\n"
"    // get the four vertices passed to the shader:\n"
"    vec2 p0 = toScreenSpace( gl_in[0].gl_Position );    // start of previous segment\n"
"    vec2 p1 = toScreenSpace( gl_in[1].gl_Position );    // end of previous segment, start of current segment\n"
"    vec2 p2 = toScreenSpace( gl_in[2].gl_Position );    // end of current segment, start of next segment\n"
"    vec2 p3 = toScreenSpace( gl_in[3].gl_Position );    // end of next segment\n"
"    // perform naive culling\n"
"    //vec2 area = WIN_SCALE * 1.2;\n"
"    //if( p1.x < -area.x || p1.x > area.x ) return;\n"
"    //if( p1.y < -area.y || p1.y > area.y ) return;\n"
"    //if( p2.x < -area.x || p2.x > area.x ) return;\n"
"    //if( p2.y < -area.y || p2.y > area.y ) return;\n"
"    // determine the direction of each of the 3 segments (previous, current, next)\n"
"    vec2 v0 = normalize( p1 - p0 );\n"
"    vec2 v1 = normalize( p2 - p1 );\n"
"    vec2 v2 = normalize( p3 - p2 );\n"
"    // determine the normal of each of the 3 segments (previous, current, next)\n"
"    vec2 n0 = vec2( -v0.y, v0.x );\n"
"    vec2 n1 = vec2( -v1.y, v1.x );\n"
"    vec2 n2 = vec2( -v2.y, v2.x );\n"
"    // determine miter lines by averaging the normals of the 2 segments\n"
"    vec2 miter_a = normalize( n0 + n1 );    // miter at start of current segment\n"
"    vec2 miter_b = normalize( n1 + n2 );    // miter at end of current segment\n"
"    // determine the length of the miter by projecting it onto normal and then inverse it\n"
"    float length_a = THICKNESS / dot( miter_a, n1 );\n"
"    float length_b = THICKNESS / dot( miter_b, n1 );\n"
"    // prevent excessively long miters at sharp corners\n"
"    if( dot( v0, v1 ) < -MITER_LIMIT ) {\n"
"        miter_a = n1;\n"
"        length_a = THICKNESS;\n"
"        // close the gap\n"
"        if( dot( v0, n1 ) > 0 ) {\n"
"            VertexOut.mTexCoord = vec2( 0, 0 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( ( p1 + THICKNESS * n0 ) / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            VertexOut.mTexCoord = vec2( 0, 0 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( ( p1 + THICKNESS * n1 ) / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            VertexOut.mTexCoord = vec2( 0, 0.5 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( p1 / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            EndPrimitive();\n"
"        }\n"
"        else {\n"
"            VertexOut.mTexCoord = vec2( 0, 1 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( ( p1 - THICKNESS * n1 ) / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            VertexOut.mTexCoord = vec2( 0, 1 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( ( p1 - THICKNESS * n0 ) / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            VertexOut.mTexCoord = vec2( 0, 0.5 );\n"
"            VertexOut.mColor = VertexIn[1].mColor;\n"
"            gl_Position = vec4( p1 / WIN_SCALE, 0.0, 1.0 );\n"
"            EmitVertex();\n"
"            EndPrimitive();\n"
"        }\n"
"    }\n"
"    if( dot( v1, v2 ) < -MITER_LIMIT ) {\n"
"        miter_b = n1;\n"
"        length_b = THICKNESS;\n"
"    }\n"
"    // generate the triangle strip\n"
"    VertexOut.mTexCoord = vec2( 0, 0 );\n"
"    VertexOut.mColor = VertexIn[1].mColor;\n"
"    gl_Position = vec4( ( p1 + length_a * miter_a ) / WIN_SCALE, 0.0, 1.0 );\n"
"    EmitVertex();\n"
"    VertexOut.mTexCoord = vec2( 0, 1 );\n"
"    VertexOut.mColor = VertexIn[1].mColor;\n"
"    gl_Position = vec4( ( p1 - length_a * miter_a ) / WIN_SCALE, 0.0, 1.0 );\n"
"    EmitVertex();\n"
"    VertexOut.mTexCoord = vec2( 0, 0 );\n"
"    VertexOut.mColor = VertexIn[2].mColor;\n"
"    gl_Position = vec4( ( p2 + length_b * miter_b ) / WIN_SCALE, 0.0, 1.0 );\n"
"    EmitVertex();\n"
"    VertexOut.mTexCoord = vec2( 0, 1 );\n"
"    VertexOut.mColor = VertexIn[2].mColor;\n"
"    gl_Position = vec4( ( p2 - length_b * miter_b ) / WIN_SCALE, 0.0, 1.0 );\n"
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


LineSprite::LineSprite(ds::ui::SpriteEngine& eng)
  : ds::ui::Sprite(eng)
  , mNeedsVboUpdate(false)
  , mLineWidth(1.0f)
  , mMiterLimit(0.5f)
  , mSmoothSpline(false) {
    mBlobType = BLOB_TYPE;
    setTransparent(false);

    // Load shaders
    try {
        mShader = ci::gl::GlslProg::create(lineVert, lineFrag, lineGeom);
    } catch (const std::exception& e) {
        std::cout << "Could not compile shader:" << e.what() << std::endl;
        return;
    }

    mLayoutFixedAspect = true;
}

void LineSprite::addPoint(const ci::vec2 point) {
	mPoints.push_back(point);
    mNeedsBatchUpdate = true;
    markAsDirty(POINTS_DIRTY);
}

void LineSprite::setPoints(const std::vector<ci::vec2>& points) {
    mPoints           = points;
    mNeedsBatchUpdate = true;
    markAsDirty(POINTS_DIRTY);
}

void LineSprite::clearPoints() {
    mPoints.clear();
    mNeedsBatchUpdate = true;
    markAsDirty(POINTS_DIRTY);
}

void LineSprite::setLineWidth(const float linewidth) {
	mLineWidth = linewidth;
    mNeedsBatchUpdate = true;
    markAsDirty(LINE_WIDTH_DIRTY);
}

void LineSprite::setSplineSmoothing(const bool doSmooth) {
	mSmoothSpline = doSmooth;
    mNeedsBatchUpdate = true;
    markAsDirty(SMOOTH_DIRTY);
}


void LineSprite::setMiterLimit(const float miterLimit) {
	mMiterLimit = miterLimit;
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

    std::vector<ci::vec3> vertices;
    // to improve performance, make room for the vertices + 2 adjacency vertices
    vertices.reserve(localPoints.size() + 2);

    // first, add an adjacency vertex at the beginning
    vertices.push_back(2.0f * ci::vec3(localPoints[0], 0) - ci::vec3(localPoints[1], 0));
    // next, add all 2D points as 3D vertices
    for (const auto& p : localPoints) vertices.push_back(ci::vec3(p, 0));

    // next, add an adjacency vertex at the end
    size_t n = localPoints.size();
    vertices.push_back(2.0f * ci::vec3(localPoints[n - 1], 0) - ci::vec3(localPoints[n - 2], 0));

    // now that we have a list of vertices, create the index buffer
    n = vertices.size() - 2;
    std::vector<uint16_t> indices;
    indices.reserve(n * 4);

    for (size_t i = 1; i < vertices.size() - 2; ++i) {
        indices.push_back(i - 1);
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // finally, create the mesh
    ci::gl::VboMesh::Layout layout;
    layout.attrib(ci::geom::POSITION, 3);

    mVboMesh = ci::gl::VboMesh::create(vertices.size(), GL_LINES_ADJACENCY_EXT, {layout}, indices.size());
    mVboMesh->bufferAttrib(ci::geom::POSITION, vertices.size() * sizeof(ci::vec3), vertices.data());
    mVboMesh->bufferIndices(indices.size() * sizeof(uint16_t), indices.data());
}

void LineSprite::buildRenderBatch() {
	if (!mNeedsBatchUpdate) return;
	mNeedsBatchUpdate = false;

	if (mPoints.size() < 3) {
		mRenderBatch.reset();
	} else {
		buildVbo();
		if (mShader && mVboMesh) {
			mShader->uniform("WIN_SCALE", ci::vec2(mEngine.getWorldWidth(), mEngine.getWorldHeight()));
			mShader->uniform("MITER_LIMIT", mMiterLimit);
			mShader->uniform("THICKNESS", mLineWidth);
			mShader->uniform("useTexture", false);

			mRenderBatch = ci::gl::Batch::create(mVboMesh, mShader);
		}
	}
}

void LineSprite::onBuildRenderBatch() {}

void LineSprite::onUpdateServer(const ds::UpdateParams& updateParams) { }

void LineSprite::drawLocalClient() {
    if (mRenderBatch && mPoints.size() >= 3) {
        mRenderBatch->draw();
    }
}

void LineSprite::writeAttributesTo(ds::DataBuffer& buf) {
	Sprite::writeAttributesTo(buf);

	const DirtyState& MITER_LIMIT_DIRTY = INTERNAL_B_DIRTY;
	const DirtyState& LINE_WIDTH_DIRTY = INTERNAL_C_DIRTY;

	const char POINTS_ATT = 80;
	const char MITER_ATT = 81;
	const char LINE_WIDTH_ATT = 82;
	if (mDirty.has(POINTS_DIRTY)) {
		buf.add(POINTS_ATT);
		buf.add((size_t)mPoints.size());
		for (const auto &p : mPoints) {
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
}

void LineSprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == POINTS_ATT) {
		size_t numPoints = buf.read<size_t>();
		for (int i = 0; i < numPoints; ++i) {
			mPoints.push_back(buf.read<ci::vec2>());
		}
		mNeedsBatchUpdate = true;
	} else if (attributeId == MITER_ATT) {
		mMiterLimit = buf.read<float>();
		mNeedsBatchUpdate = true;

	} else if (attributeId == SMOOTH_ATT) {
		mSmoothSpline = buf.read<bool>();
		mNeedsBatchUpdate = true;
	} else if (attributeId == LINE_WIDTH_ATT) {
		mLineWidth = buf.read<float>();
		mNeedsBatchUpdate = true;
	} else {
		Sprite::readAttributeFrom(attributeId, buf);
	}
}

}  // namespace experiments
}  // namespace experiments
