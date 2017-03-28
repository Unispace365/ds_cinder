#version 150

uniform sampler2D   tex0;
uniform bool        useTexture;
uniform bool        preMultiply;

in vec4             Color;
in vec2             TexCoord0;

out vec4            oColor;

void main()
{
    oColor = vec4(1.0, 1.0, 1.0, 1.0);

    if (useTexture) {
        oColor = texture2D( tex0, TexCoord0 );
    }

    oColor *= Color;

    if (preMultiply) {
        oColor.r *= Color.a;
        oColor.g *= Color.a;
        oColor.b *= Color.a;
    }
}