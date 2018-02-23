uniform sampler2D  tex0;
uniform bool       useTexture;
uniform bool       preMultiply;
uniform vec2      touchPos;
uniform float 		iTime;
in vec2            TexCoord0;
in vec4            Color;
out vec4           oColor;


vec3 iResolution = vec3(1920,1080,1);

// license: "public domain, but I appreciate credits mentions if you use this :)"
#define BLOCK_SIZE 28.0
#define BLOCK_CTRL 0.1

float sat( float t ) {
	return clamp( t, 0.0, 1.0 );
}
vec2 sat( vec2 t ) {
	return clamp( t, 0.0, 1.0 );
}
float remap  ( float t, float a, float b ) {
	return sat( (t - a) / (b - a) );
}
float linterp( float t ) {
	return sat( 1.0 - abs( 2.0*t - 1.0 ) );
}
// this function and the ones above aren't mine
// they aren't really needed but I don't feel like
// removing them either
// I don't remember where they were from...
vec3 spectrum_offset( float t ) {
	vec3 ret;
	float lo = step(t,0.5);
	float hi = 1.0-lo;
	float w = linterp( remap( t, 1.0/6.0, 5.0/6.0 ) );
	float neg_w = 1.0-w;
	ret = vec3(lo,1.0,hi) * vec3(neg_w, w, neg_w);
	return pow( ret, vec3(1.0/2.2) );
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoord0.xy; // iResolution.xy;
    uv.y *= iResolution.y/iResolution.x;
	float time = iTime;
    vec4 sum = texture(tex0, vec2(1.0,1.778)*uv);

    const float amount = 6.0;
    for(float i = 0.0; i < amount; i++){
        uv /= pow(mix(vec2(1.0), fract((touchPos.x / 90.0 + 100.0)*uv)+0.5, clamp(pow(length(texture(tex0, vec2(0.06*iTime)).xyz ), 15.0),0.0, 1.0)), vec2(BLOCK_CTRL)); 
        sum = clamp(sum, 0.15, 1.0);
        sum /= 0.1+0.9*clamp(texture(tex0, vec2(1.0,1.778)*uv+vec2(1.0/iResolution.x,i/iResolution.y)),0.0,2.0);
    	sum *= 0.1+0.9*clamp(texture(tex0, vec2(1.0,1.778)*uv+vec2(1.0/iResolution.x,-i/iResolution.y)),0.0,2.0);
    	sum *= 0.1+0.9*clamp(texture(tex0, vec2(1.0,1.778)*uv+vec2(-i/iResolution.x,i/iResolution.y)),0.0,2.0);
    	sum /= 0.1+0.9*clamp(texture(tex0, vec2(1.0,1.778)*uv+vec2(-i/iResolution.x,-i/iResolution.y)),0.0,2.0);
        sum.xyz /= 1.01-0.025*spectrum_offset( 1.0-length(sum.xyz) );
    	sum.xyz *= 1.0+0.01*spectrum_offset( length(sum.xyz) );
        
    }
    sum = 0.1+0.9*sum;
    float chromaf = pow(length(texture(tex0, vec2(0.0213*iTime)).xyz ), 2.0);
    sum /= length(sum);
    sum = (-0.2+2.0*sum)*0.9;
	oColor = sum;
}
