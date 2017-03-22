#version 150

uniform sampler2D	tex0;
uniform float		opacity;

in vec2            TexCoord0;
in vec4            Color;
out vec4           oColor;

void main()
{ 

 
vec4 myColor = texture2D( tex0, TexCoord0 );
myColor = myColor *vec4(1.0,0.0,0.0,1.0);
myColor.a *= opacity;
oColor = myColor;

}