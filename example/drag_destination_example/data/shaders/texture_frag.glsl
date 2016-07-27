#version 110

uniform sampler2D	tex0;
uniform float	opacity;

void main()
{ 


	vec2 flip = vec2( gl_TexCoord[0].s ,  gl_TexCoord[0].t );
	vec4 sample = texture2D( tex0 , gl_TexCoord[0].st );
	sample.a *= sample.a;
	sample.a *= opacity;
	gl_FragColor = sample;
	

	
}