precision highp float;
uniform sampler2D gsuTexture0;
uniform sampler2D gsuTexture1;
uniform sampler2D gsuTexture2;
varying vec2 gsvTexCoordLuma;
varying vec2 gsvTexCoordChroma;

void main()
{
	float y = texture2D(gsuTexture0, gsvTexCoordLuma).r;
	float u = texture2D(gsuTexture1, gsvTexCoordChroma).r;
	float v = texture2D(gsuTexture2, gsvTexCoordChroma).r;
	u = u - 0.5;
	v = v - 0.5;
	vec3 rgb;
	rgb.r = y + (1.403 * v);
	rgb.g = y - (0.344 * u) - (0.714 * v);
	rgb.b = y + (1.770 * u);
	gl_FragColor = vec4(rgb, 1.0);
}