#include "stdafx.h"

#include "ds/ui/sprite/circle_border.h"

#include <ds/app/environment.h>

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace {

const std::string CIRCLE_BORDER_FRAG = "#version 150\n"

"in vec2 position_interpolated;"
"in vec2 texture_interpolated;"
"in vec2 extent_interpolated;"
"in vec4 extra_interpolated;"
""
"uniform sampler2D tex0;"
"uniform bool useTexture;"
"uniform bool preMultiply;"

"in vec2            TexCoord0;"
"in vec4            Color;"
"out vec4           oColor;"

"void main()"
"{"
"	oColor = vec4(1.0, 1.0, 1.0, 1.0);"
""
""
"	if(useTexture) {"
"		oColor = texture2D(tex0, TexCoord0);"
"	}"
""
"	oColor *= Color;"
""
"	if(preMultiply) {"
"		oColor.r *= oColor.a;"
"		oColor.g *= oColor.a;"
"		oColor.b *= oColor.a;"
"	}"
""
"	vec2 circleExtent = extent_interpolated;"
"	vec2 circleCenter = circleExtent * 0.5;"
"	vec2 delta = position_interpolated - circleCenter;"
""
"	// apply the general equation of an ellipse (outer edge)"
"	vec2 outerRadius = circleExtent * 0.5;"
"	float outerDistance = ("
"		((delta.x * delta.x) / (outerRadius.x * outerRadius.x)) +"
"		((delta.y * delta.y) / (outerRadius.y * outerRadius.y))"
"		);"
""
"	// apply the general equation of an ellipse (inner edge)"
"	vec2 innerRadius = outerRadius - vec2(extra_interpolated.x, extra_interpolated.x);"
"	float innerDistance = ("
"		((delta.x * delta.x) / (innerRadius.x * innerRadius.x)) +"
"		((delta.y * delta.y) / (innerRadius.y * innerRadius.y))"
"		);"
""
"	float totalAlpha;"
""
"	// do this with minimal aliasing"
"	float outerFragDelta = fwidth(outerDistance) * 3.0;"
"	float outerAlpha = 1.0 - smoothstep(1.0 - outerFragDelta, 1.0, outerDistance);"
"	float innerFragDelta = fwidth(innerDistance) * 2.0;"
"	float innerAlpha = smoothstep(1.0 - innerFragDelta, 1.0, innerDistance);"
""
"	totalAlpha = outerAlpha * innerAlpha;"
""
"	oColor.a *= totalAlpha;"
"}";

const std::string CIRCLE_BORDER_VERT = "#version 150\n"

"out vec2 			position_interpolated;"
"out vec2 			texture_interpolated;"
"out vec4 			extra_interpolated;"
"out vec2 			extent_interpolated;"
""
"uniform bool 		useTexture;"
"uniform vec2 		extent;"
"uniform vec4 		extra;"
"uniform mat4		ciModelMatrix;"
"uniform mat4		ciModelViewProjection;"
"uniform vec4		uClipPlane0;"
"uniform vec4 		uClipPlane1;"
"uniform vec4		uClipPlane2;"
"uniform vec4		uClipPlane3;"
""
"in vec4			ciPosition;"
"in vec2			ciTexCoord0;"
"in vec4 			ciColor;"
"out vec2			TexCoord0;"
"out vec4			Color;"
""
"void main()"
"{"
"	position_interpolated = ciPosition.xy;"
"	if(useTexture) {"
"		texture_interpolated = texture_interpolated;"
"	}"
""
"	extent_interpolated = extent;"
"	extra_interpolated = extra;"
""
"	gl_Position = ciModelViewProjection * ciPosition;"
"	TexCoord0 = ciTexCoord0;"
"	Color = ciColor;"
""
"	gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);"
"	gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);"
"	gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);"
"	gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);"
"}";
}

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
	initialize();
}

CircleBorder::CircleBorder(SpriteEngine& engine, const float width)
	: inherited(engine)
	, mBorderWidth(width)
{
	initialize();
}

void CircleBorder::initialize(){
	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setBaseShader(CIRCLE_BORDER_VERT, CIRCLE_BORDER_FRAG, "circle_border_shaders");
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
