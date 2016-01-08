uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

void main()
{
	vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

    if (useTexture) {
        color = texture2D( tex0, gl_TexCoord[0].st );
    }
    
    color *= gl_Color;
    
    if (preMultiply) {
        color.r *= color.a;
        color.g *= color.a;
        color.b *= color.a;
    }    
    
    vec2 centerToFrag = vec2(gl_TexCoord[0].st - vec2(0.5, 0.5));
	float centerDistanceSquared = dot(centerToFrag, centerToFrag);
	
	// this should be smoothed out
	float squareDistanceBeyondEdge = (centerDistanceSquared - (0.5 * 0.5));
	float insideAlpha = mix(1.0, 0.0, clamp(squareDistanceBeyondEdge * 100.0, 0.0, 1.0));
	
	color.a *= insideAlpha;
	
	gl_FragColor = color;
}