#version 120

varying vec2 position_interpolated;
varying vec2 texture_interpolated;

uniform bool useTexture;

attribute vec2 extent;
varying vec2 extent_interpolated;

attribute vec4 extra;
varying vec4 extra_interpolated;

void main()
{
    position_interpolated = gl_Vertex.xy;
	if(useTexture) {
		texture_interpolated = texture_interpolated;
	}
	
	extent_interpolated = extent;
	extra_interpolated = extra;

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
}
