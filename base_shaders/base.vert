#version 150

uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
in vec4			ciColor;
out vec4		Color;
in vec2			ciTexCoord0;
out vec2		TexCoord0;

void main()
{
    gl_Position = ciModelViewProjection * ciPosition;
 //   gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
   TexCoord0 = ciTexCoord0;

    Color = ciColor;
}
