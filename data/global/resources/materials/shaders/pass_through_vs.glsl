// Vertex Shader 
// Espacilly designed for use with ambient passes
// Passes vertex position to Pixel shader
// Passes uv coordinates to Pixel shader
 
#version 140
 
uniform mat4 modelViewProj;

in vec4 vertex;
in vec4 uv0;

out vec4 uv;

void main(void)
{
	gl_Position = modelViewProj * vertex;

	uv = uv0;
}

