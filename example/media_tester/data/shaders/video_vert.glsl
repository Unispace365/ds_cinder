attribute vec2 gsaTexCoord;
varying   vec2 gsvTexCoord;
varying   vec2 gsvTexCoordLuma;
varying   vec2 gsvTexCoordChroma;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
	
	gsvTexCoord = gl_TexCoord[0]; //gsaTexCoord;
	gsvTexCoordLuma.s = gl_TexCoord[0].s;
	gsvTexCoordLuma.t = gl_TexCoord[0].t;
	gsvTexCoordChroma.s = gl_TexCoord[0].s;// / 4.0;
	gsvTexCoordChroma.t = gl_TexCoord[0].t;// / 8.0;
}