#version 150

in vec2 position_interpolated;
in vec2 texture_interpolated;
in vec2 extent_interpolated;
in vec4 extra_interpolated;

uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

in vec2            TexCoord0;
in vec4            Color;
out vec4           oColor;

void main()
{
	oColor = vec4(1.0, 1.0, 1.0, 1.0);


    if (useTexture) {
        oColor = texture2D( tex0, TexCoord0 );
    }
    
    oColor *= Color;
    
    if (preMultiply) {
        oColor.r *= oColor.a;
        oColor.g *= oColor.a;
        oColor.b *= oColor.a;
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
	
	oColor.a *= totalAlpha;
	
}