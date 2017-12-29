#version 150

out vec2 			position_interpolated;
out vec2 			texture_interpolated;
out vec4 			extra_interpolated;
out vec2 			extent_interpolated;

uniform bool 		useTexture;
uniform vec2 		extent;
uniform vec4 		extra;
uniform mat4		ciModelMatrix;
uniform mat4		ciModelViewProjection;
uniform vec4		uClipPlane0;
uniform vec4 		uClipPlane1;
uniform vec4		uClipPlane2;
uniform vec4		uClipPlane3;

in vec4				ciPosition;
in vec2				ciTexCoord0;
in vec4 			ciColor;
out vec2			TexCoord0;
out vec4			Color;

void main()
{
    position_interpolated = ciPosition.xy;
	if(useTexture) {
		texture_interpolated = texture_interpolated;
	}
	
	extent_interpolated = extent;
	extra_interpolated = extra;

    gl_Position = ciModelViewProjection * ciPosition;
    TexCoord0 = ciTexCoord0;
    Color = ciColor;
	
    gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);
    gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);
    gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);
    gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);
}



