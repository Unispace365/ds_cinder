uniform sampler2D tex0;
uniform bool useTexture;
uniform bool preMultiply;

varying vec3 normal;
varying vec3 v;
uniform vec4 camPos;

const float ambient_light = 0.15;
const float gamma = 1.0;

const vec3  rimColor = vec3(0.8, 0.85, 1.0);
const float rimStart = 0.35;
const float rimEnd = 0.95;
const float rimMultiplier = 0.83;


void main(void)
{
    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

    if (useTexture) {
        color = texture2D( tex0, gl_TexCoord[0].st );
    }

    color *= gl_Color;

	//vec3 light_dir = normalize(gl_LightSource[0].position.xyz - v);
	vec3 light_dir = normalize( vec3( -60, 40, 50 ) );
	float diffuse_light = max( dot(normal, light_dir), 0.0 );
	diffuse_light = clamp(diffuse_light, 0.0, 0.8);

	vec3 diffuse_color = color.rgb * diffuse_light;
	vec3 ambient_color = color.rgb * ambient_light;

	vec3 linear_color = diffuse_color*(1.0-ambient_light) + ambient_color;

	// Rimlight Shader
	float NormalToCam = 1.0 - dot(normalize(normal), normalize(camPos-v));
	float rim = smoothstep(rimStart, rimEnd, NormalToCam) * rimMultiplier;
	rim *= clamp( diffuse_light, 0.6, 1.0 );
	linear_color.rgb += rim*rimColor;

	gl_FragColor = vec4( pow( linear_color, vec3(1.0/gamma) ), color.a );
}

