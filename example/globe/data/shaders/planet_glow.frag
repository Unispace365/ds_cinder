uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

uniform vec4 size;
varying vec4 p_o;

void main()
{
	//vec2 p = p_o.xy / float(size.xy);
	//vec2 center = fsize / 2.0;

	vec2 p = gl_TexCoord[0].st;
	vec2 center = vec2(0.5, 0.5);
    //vec2 fsize = vec2( size.xy );

	float glow = smoothstep( 0.0, 1.0, 1.0 - (length(p - center) * 2.0)); // / (fsize/2.0);
	//vec2 c = pow( cos((p-center)*3.14159265), 3 );
	//float glow = c.x*c.y;
	//float glow = length(p);

    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
    if (useTexture) {
        color = texture2D( tex0, gl_TexCoord[0].st );
    }
    color *= gl_Color;
    color.a *= glow;
    
    if (preMultiply) {
        color.rgb *= color.a;
    }    
    
    gl_FragColor = color;
    //gl_FragColor = vec4(1);
}
