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
    
	vec2 circleExtent = extent_interpolated;
	vec2 circleCenter = circleExtent * 0.5;
	vec2 delta = position_interpolated - circleCenter;
	
	// apply the general equation of an ellipse (outer edge)
	vec2 outerRadius = circleExtent * 0.5;
	float outerDistance = (
		((delta.x * delta.x) / (outerRadius.x * outerRadius.x)) +
		((delta.y * delta.y) / (outerRadius.y * outerRadius.y))
	);
	
	// apply the general equation of an ellipse (inner edge)
	vec2 innerRadius = outerRadius - vec2(extra_interpolated.x, extra_interpolated.x);
	float innerDistance = (
		((delta.x * delta.x) / (innerRadius.x * innerRadius.x)) +
		((delta.y * delta.y) / (innerRadius.y * innerRadius.y))
	);
	
	float totalAlpha;
	
	// do this with minimal aliasing
	float outerFragDelta = fwidth(outerDistance) * 3.0;
	float outerAlpha = 1.0 - smoothstep(1.0 - outerFragDelta, 1.0, outerDistance);
	float innerFragDelta = fwidth(innerDistance) * 2.0;
	float innerAlpha = smoothstep(1.0 - innerFragDelta, 1.0, innerDistance);
	
	totalAlpha = outerAlpha * innerAlpha;
	
	/*
	// apply the general equation of an ellipse (middle of border)
	vec2 middleRadius = outerRadius - vec2(extra_interpolated.x * 0.5, extra_interpolated.x * 0.5);
	float middleDistance = (
		((delta.x * delta.x) / (middleRadius.x * middleRadius.x)) +
		((delta.y * delta.y) / (middleRadius.y * middleRadius.y))
	);
	
	// I'd like to find a way to smooth this out without the pow functions, and without the ternary operator
	float powOuter = pow(outerDistance, 50.0);
	float powInner = pow(innerDistance, 50.0);
	
	totalAlpha = (middleDistance < 1.0) ? powInner : (1.0 - powOuter);
	*/
	
	/*
	// you can use this instead if you want a hard edge
	// for now, just provide a hard edge
	float outerAlpha = step(outerDistance, 1.0);
	float innerAlpha = 1.0 - step(innerDistance, 1.0);
	totalAlpha = outerAlpha * innerAlpha;
	*/
	
	color.a *= totalAlpha;
	
	gl_FragColor = color;
}