// Fragment Shader 
// Flat diffuse texture
 
#version 140

uniform sampler2D diffuseTexture;
 
in vec4 uv;

out vec4 FragmentColour;
 
void main(void)
{
	FragmentColour = texture2D(diffuseTexture, uv.xy);
}

