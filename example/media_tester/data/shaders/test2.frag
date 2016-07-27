#version 110

uniform sampler2D	tex0;
uniform float		opacity;

varying vec4 vertTexcoord;

void main()
{ 

 
vec4 myColor = texture2D( tex0, vertTexcoord.xy);
myColor = myColor *vec4(1.0,0.0,0.0,1.0);
myColor.a *= opacity;
gl_FragColor = myColor;

}