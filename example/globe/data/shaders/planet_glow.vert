#version 150

uniform mat4 ciModelViewProjection;
uniform mat3 ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciNormal;
in vec2 ciTexCoord0;
in vec4 ciColor;

out vec3 vertPosition;
out vec3 vertNormal;
out vec2 vertTexCoord0;
out vec4 vertColor;

void main()
{
	vertPosition = ciPosition.xyz;
	vertNormal = ciNormalMatrix * ciNormal;
	vertTexCoord0 = ciTexCoord0;
	vertColor = ciColor;

	gl_Position = ciModelViewProjection * ciPosition;
}
