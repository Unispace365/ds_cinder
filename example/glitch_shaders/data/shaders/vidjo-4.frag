uniform sampler2D  tex0;
uniform bool       useTexture;
uniform bool       preMultiply;
uniform vec2      touchPos;
uniform float 		iTime;
in vec2            TexCoord0;
in vec4            Color;
out vec4           oColor;


vec3 iResolution = vec3(1920,1080,1);


void main( )
{
    float warpSpeed = 0.3;
    float warpDistance = 0.02;
    
	vec2 uv = TexCoord0;// iResolution.xy;
    //uv.y *= -1.0;
    
    vec3 offTexX = texture(tex0, uv).rgb;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float power = dot(offTexX, luma);
    
    power = sin(3.1415927*2.0 * mod(power + iTime * warpSpeed, 1.0));
    
    oColor = texture(tex0, uv+vec2(0, power)*warpDistance);
    
}
