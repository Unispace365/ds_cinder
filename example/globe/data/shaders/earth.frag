#version 150
#define lerp mix

uniform float counter;

uniform sampler2D texDiffuse;
uniform sampler2D texDiffuseNight;
uniform sampler2D texNormal;
uniform sampler2D texMask;
uniform float time;

uniform vec3 lightDir;

in vec3 vertPosition;
in vec3 vertNormal;
in vec2 vertTexCoord0;

out vec4 fragColor;

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( vec3 x )
{
    // The noise function returns a value in the range -1.0f -> 1.0f
    vec3 p = floor(x);
    vec3 f = fract(x);

    f       = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;

    //Sure I guess, do this shit.
    return lerp(lerp(lerp( hash(n+0.0), hash(n+1.0),f.x),lerp( hash(n+57.0), hash(n+58.0),f.x),f.y),
           lerp(lerp( hash(n+113.0), hash(n+114.0),f.x),
           lerp( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}


void main()
{



	vec2 texCoord			= vec2( vertTexCoord0.s, vertTexCoord0.t );
	vec3 diffuseSample		= texture( texDiffuse, texCoord ).rgb;
	vec3 nightSample 		= texture( texDiffuseNight, texCoord ).rgb;
	vec3 normalSample		= texture( texNormal, texCoord ).rgb * 2.0 - 1.0;
	vec3 texSample			= texture( texMask, texCoord ).rgb;
	
	float night = 0.5 + sin( time * 0.08 + (texCoord.y + texCoord.x * 3.14159 * 2.0) ) * 0.5;
	night = smoothstep( 0.4,0.52,  night);
	
	// use green channel for land elevation data
	float landValue			= texSample.g;

	// use blue channel for ocean elevation data
	float oceanValue		= texSample.b;

	
	vec3 ppNormal			= normalize( vertNormal + normalSample );
	float ppDiffuse			= abs( dot( ppNormal, lightDir ) );
	float ppFresnel			= pow( ( 1.0 - ppDiffuse ), 3.0 );
	
	float lrayNscale = 634.0;
	float xn = 0.5 - smoothstep( 0.0, 0.16, noise(vec3( time * 0.3 + texCoord.x * lrayNscale, lrayNscale * texCoord.y , time*5.5)));
	float yn = 0.5 - smoothstep( 0.0, 0.16, noise(vec3(texCoord.x * lrayNscale,lrayNscale *  texCoord.y,texCoord.x * 123.0+ texCoord.y *43.0 +  time*0.5)));

	vec3 nNormal =  normalize( vec3( xn, yn, 0.0 ));
	
	float lightRay			= smoothstep( 0.94, 1.0, abs( dot( ppNormal + nNormal, vec3(0.0,0.1,1.0) ) ));
	float ppSpecular		= pow( ppDiffuse, 10.0 );
	float ppSpecularBright	= pow( ppDiffuse, 120.0 );
	
	
	// use red channel for nighttime city lights
	float nscale = 1200.0;
	float etime = time * 2.0;
	
	vec3 blinkNoise = vec3(texCoord.x * nscale, texCoord.y * nscale, etime);
	float baseElectricity = texSample.r * night * night;
	float blinkElectricity= baseElectricity * noise(blinkNoise);
	float electrictyValue = 0.4 * baseElectricity + 0.6 * blinkElectricity;

	
	float elevation = (texSample.g+texSample.b)*0.2 + 0.8;
	vec3 daynight = mix(diffuseSample,nightSample,night) * ppDiffuse;
	
	vec3 landFinal			= elevation * daynight * 0.75 + vec3(1.0,1.0,0.0)*baseElectricity*0.5 + blinkElectricity * 0.3 + pow(baseElectricity,0.5)*lightRay;
	vec3 oceanFinal			= daynight * oceanValue;
	
	//lightRay *= baseElectricity;
	
	float r				= ( 1.0 - ppNormal.r ) * oceanValue * 0.5;
	fragColor.rgb		=   landFinal + 0.5 * ppSpecularBright * (smoothstep(0.20, 0.26,texSample.b));// + vec3( r*r, r * 0.25, 0 ) * oceanValue;
	fragColor.a			= 1.0;
}

