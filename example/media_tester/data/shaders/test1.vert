varying vec4 vertTexcoord;

void main()
{	
	gl_TexCoord[0] = gl_MultiTexCoord0;	
	vertTexcoord = gl_TexCoord[0];

	gl_Position = ftransform();
}