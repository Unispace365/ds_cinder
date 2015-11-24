#version 110

uniform sampler2D	tex1;
uniform vec2		sample_offset;
uniform float		attenuation;
uniform float		opacity;

varying vec4 vertTexcoord;

void main()
{ 

 
vec4 previousPass = texture2D( tex1, vertTexcoord.xy);
previousPass = previousPass + 0.5 ; 
previousPass.a *= opacity;

gl_FragData[0] = previousPass;



}