// Fragment Shader 
// Ambient lighting
// TODO add support for ambient occulusion maps
 
#version 140
 
uniform vec4 lightAmbient;
uniform vec4 surfaceAmbient;

in vec4 uv;

out vec4 FragmentColour;
 
void main(void)
{
	FragmentColour = surfaceAmbient*lightAmbient;
}

