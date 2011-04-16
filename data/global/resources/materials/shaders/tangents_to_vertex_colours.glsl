// Simple vertex shader to pass the transformed tangents as vertex colours

// Transform matrix, can be any space the user wants to show the normals
uniform mat4 transform;
uniform mat4 modelViewProj;

in vec4 vertex;
in vec3 tangent;

out vec4 vColour;

void main(void)
{
	// Vertex location
	gl_Position = modelViewProj * vertex;

	// Normal to vertex colour
	vec3 tTangent = (transform*vec4(normalize(tangent), 0.0)).xyz;
	// Normalize and abs so that we can see the -+direction
	// we could also use normalize and rescale it from [-1, 1] to [0, 1]
	vColour = vec4(abs(normalize(tTangent)), 1.0);
}

