// Fragment Shader 
// Ambient lighting
// TODO add support for ambient occulusion maps
 
#version 140
 
uniform vec4 lightAmbient;
uniform vec4 surfaceAmbient;
uniform vec4 surfaceEmissive;

in vec4 uv;

out vec4 FragmentColour;
 
void main(void)
{
	vec4 colour = surfaceAmbient*lightAmbient + surfaceEmissive;
	colour.a = surfaceAmbient.a;
	FragmentColour = clamp(colour, 0.0, 1.0);
}

