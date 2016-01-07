#version 120

varying vec2 position_interpolated;
varying vec2 texture_interpolated;
varying vec2 extent_interpolated;
varying vec4 extra_interpolated;

uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

void main()
{
	vec4 color = vec4(
		position_interpolated.x / extent_interpolated.x,
		position_interpolated.y / extent_interpolated.y,
		0,
		1
	);
	
	color = vec4(1.0, 1.0, 1.0, 1.0);

    if (useTexture) {
        color = texture2D( tex0, gl_TexCoord[0].st );
    }
    
    color *= gl_Color;
    
    if (preMultiply) {
        color.r *= color.a;
        color.g *= color.a;
        color.b *= color.a;
    }    
    
	vec2 circleExtent = vec2(
		extra_interpolated.z - extra_interpolated.x,
		extra_interpolated.w - extra_interpolated.y
	);
	vec2 circleRadius = circleExtent * 0.5;
	vec2 circleCenter = vec2(
		(extra_interpolated.x + extra_interpolated.z) * 0.5,
		(extra_interpolated.y + extra_interpolated.w) * 0.5
	);
	vec2 delta = position_interpolated - circleCenter;
	
	// apply the general equation of an ellipse
	float radialDistance = (
		((delta.x * delta.x) / (circleRadius.x * circleRadius.x)) +
		((delta.y * delta.y) / (circleRadius.y * circleRadius.y))
	);
	
	float totalAlpha;
	
	// do this with minimal aliasing
	float fragDelta = fwidth(radialDistance) * 3.0;
	totalAlpha = 1.0 - smoothstep(1.0 - fragDelta, 1.0, radialDistance);
	
	/*
	// I'd like to find a way to smooth this out without the pow function
	float powDistance = pow(radialDistance, 10.0);	
	totalAlpha = 1.0 - powDistance;
	*/
	
	/* 
	// use this if you want a hard edge
	totalAlpha = step(radialDistance, 1.0);
	*/
	
	color.a *= totalAlpha;
	
	gl_FragColor = color;
}