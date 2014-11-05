varying vec3 normal;
varying vec3 v;

uniform vec4 camPos;
uniform mat4 modelMatrix;
//uniform vec4 worldPos;

void main()
{
	//normal = normalize(gl_NormalMatrix * gl_Normal);
	//normal = normalize(modelMatrix * vec4(gl_Normal, 0.0)).xyz;
	//normal = gl_Normal.xyz;
	normal = normalize( (vec4(gl_Normal, 0) * inverse(modelMatrix)).xyz );
	v = (modelMatrix * gl_Vertex).xyz;

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
}
