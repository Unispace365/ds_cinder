uniform bool useTexture;
uniform bool preMultiply;

uniform vec4 size;
out vec4 fragColor;

in vec3 vertPosition;
in vec3 vertNormal;
in vec2 vertTexCoord0;
in vec4 vertColor;

void main()
{
	vec2 p = vertTexCoord0.st;
	vec2 center = vec2(0.5, 0.5);
    //vec2 fsize = vec2( size.xy );

	float glow = smoothstep( 0.0, 1.0, 1.0 - (length(p - center) * 2.0)); // / (fsize/2.0);
	//vec2 c = pow( cos((p-center)*3.14159265), 3 );
	//float glow = c.x*c.y;
	//float glow = length(p);

    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

    color *= vertColor;
    color.a *= glow;
    
    if (preMultiply) {
        color.rgb *= color.a;
    }    
    
    fragColor = vec4(glow);
    
}
