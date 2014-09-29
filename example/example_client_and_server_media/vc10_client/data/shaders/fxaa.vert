
varying vec4 vertColor;
varying vec4 vertTexcoord;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    vertTexcoord = gl_TexCoord[0];
    vertColor = gl_Color;

    gl_FrontColor = gl_Color;
}
