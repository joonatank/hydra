// Simple vertex shader to pass the transformed normals as vertex colours

// Transform matrix, can be any space the user wants to show the normals
uniform mat4 transform;
uniform mat4 modelViewProj;

in vec4 vertex;
in vec3 normal;

out vec4 vColour;

void main(void)
{
	// Vertex location
	gl_Position = modelViewProj * vertex;

	// Normal to vertex colour
	vec3 tNormal = (transform*vec4(normalize(normal), 0.0)).xyz;
	vColour = vec4(abs(normalize(normal)), 1.0);
}

