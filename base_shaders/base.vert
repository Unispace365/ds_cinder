#version 150
uniform mat4       ciModelMatrix;
uniform mat4       ciModelViewProjection;
uniform vec4       uClipPlane0;
uniform vec4       uClipPlane1;
uniform vec4       uClipPlane2;
uniform vec4       uClipPlane3;
in vec4            ciPosition;
in vec2            ciTexCoord0;
in vec4            ciColor;
out vec2           TexCoord0;
out vec4           Color;

void main()
{
    gl_Position = ciModelViewProjection * ciPosition;
    TexCoord0 = ciTexCoord0;
    Color = ciColor;
    gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);
    gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);
    gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);
    gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);
}