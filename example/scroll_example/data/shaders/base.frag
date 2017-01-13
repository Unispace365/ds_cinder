uniform sampler2D 	tex0;
uniform bool 		useTexture;
uniform bool		preMultiply;
in vec4				ciColor;
out vec4			oColor;
in vec2				TexCoord0;

void main()
{
    oColor = vec4(1.0, 1.0, 1.0, 1.0);

    if (useTexture) {
        oColor = texture2D( tex0, TexCoord0 );
    }
    
    oColor *= ciColor;
    
    if (preMultiply) {
        oColor.r *= oColor.a;
        oColor.g *= oColor.a;
        oColor.b *= oColor.a;
    }      
}