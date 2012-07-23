uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

void main()
{
    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
    color *= gl_Color;

    if (useTexture) {
        color = texture2D( tex0, gl_TexCoord[0].st );
    }
    
    if (preMultiply) {
        color.r *= color.a;
        color.g *= color.a;
        color.b *= color.a;
    }    
    
    gl_FragColor = color;
}