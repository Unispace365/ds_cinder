uniform vec4 size;
varying vec4 p_o;

void main()
{
	p_o  = gl_Vertex;

	//vec3 v = gl_Vertex.xyz - vec3( 0.5, 0.5, 0 );
	vec3 v = vec3(gl_Vertex.xy, 0 ) - vec3( size.xyz / 2.0 ); // * 1.5;
	gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0) + vec4(v, 0.0));

    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
}
