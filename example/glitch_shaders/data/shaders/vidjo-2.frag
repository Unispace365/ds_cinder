uniform sampler2D  tex0;
uniform bool       useTexture;
uniform bool       preMultiply;
uniform vec2      touchPos;
uniform float 		iTime;
in vec2            TexCoord0;
in vec4            Color;
out vec4           oColor;


vec3 iResolution = vec3(1920,1080,1);

float GlitchAmount = 1.0;

vec4 posterize(vec4 color, float numColors)
{
    return floor(color * numColors - 0.5) / numColors;
}

vec2 quantize(vec2 v, float steps)
{
    return floor(v * steps) / steps;
}

float dist(vec2 a, vec2 b)
{
    return sqrt(pow(b.x - a.x, 2.0) + pow(b.y - a.y, 2.0));
}

void main()
{   
	GlitchAmount = 2.0 * touchPos.x / iResolution.x + 0.75;
	vec2 uv = TexCoord0.xy;// / iResolution.xy;
    float amount = pow(GlitchAmount, 2.0);
    vec2 pixel = 1.0 / iResolution.xy;    
    vec4 color = texture(tex0, uv);
    if (TexCoord0.x < touchPos.x)
    {
    //    oColor = color;
    //    return;
    }
    float t = mod(mod(iTime, amount * 100.0 * (amount - 0.5)) * 109.0, 1.0);
    vec4 postColor = posterize(color, 16.0);
    vec4 a = posterize(texture(tex0, quantize(uv, 64.0 * t) + pixel * (postColor.rb - vec2(.5)) * 100.0), 5.0).rbga;
    vec4 b = posterize(texture(tex0, quantize(uv, 32.0 - t) + pixel * (postColor.rg - vec2(.5)) * 1000.0), 4.0).gbra;
    vec4 c = posterize(texture(tex0, quantize(uv, 16.0 + t) + pixel * (postColor.rg - vec2(.5)) * 20.0), 16.0).bgra;
    oColor = mix(
        			texture(tex0, 
                              uv + amount * (quantize((a * t - b + c - (t + t / 2.0) / 10.0).rg, 16.0) - vec2(.5)) * pixel * 100.0),
                    (a + b + c) / 3.0,
                    (0.5 - (dot(color, postColor) - 1.5)) * amount);
}