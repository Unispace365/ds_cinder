#version 120

varying vec2 position_interpolated;
varying vec2 texture_interpolated;

uniform bool useTexture;

void main()
{
	position_interpolated = gl_Vertex.xy;
	if(useTexture) {
		texture_interpolated = texture_interpolated;
	}

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
}
