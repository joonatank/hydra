// Fragment Shader 
// Ambient lighting
// TODO add support for light maps (emissive channel)
 
#version 140
 
uniform vec4 lightAmbient;
uniform vec4 surfaceAmbient;
uniform vec4 surfaceEmissive;

#ifdef AMBIENT_OCCLUSION
uniform sampler2D aoMap;
#endif

in vec4 uv;

out vec4 FragmentColour;
 
void main(void)
{
#ifdef AMBIENT_OCCLUSION
	vec4 aoColour = texture2D(aoMap, uv.xy);
	vec4 ambient = surfaceAmbient*aoColour*lightAmbient;
#else
	vec4 ambient = surfaceAmbient*lightAmbient;
#endif
	vec4 colour = ambient + surfaceEmissive;

	colour.a = surfaceAmbient.a;
	FragmentColour = clamp(colour, 0.0, 1.0);
}

