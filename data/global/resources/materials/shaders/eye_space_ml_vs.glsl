// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Eye space vertex program for single light.

// Defines:
// These values must correspond to the ones in pixel shader.
// SHADOW_MAP : Shadows can be turned on/off.


#version 140

uniform mat4 modelView;
uniform mat4 modelViewProj;
// Light position in eye space
uniform vec4 mvLightPos[MAX_LIGHTS];
uniform vec4 spotDirection[MAX_LIGHTS];
uniform float nLights;

#ifdef SHADOW_MAP
uniform mat4 model;
// Shadow texture transform
uniform mat4 texViewProj;
#endif

// The order of these is significant
// my guess is that uv0 can not appear after tangent and binormal (uv6 and uv7)
// Shouldn't matter any more, but we can check (fix to Ogre GLSL parser)
in vec4 vertex;
in vec3 normal;
in vec4 uv0;

out vec4 uv;

out vec3 vNormal;

// from vertex to eye in eye space
out vec3 dirToEye;
// Eye space light position for attenuation and spotlight
// For low poly object this needs to pass through to get the correct
// interpolated values for fragment.
out vec3 lightPos[MAX_LIGHTS];
// Spotlight direction vector in eye space
out vec3 spotlightDir[MAX_LIGHTS];
// Vertex in eye space for calculating light direction
out vec3 vVertex;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
#ifdef SHADOW_MAP
out vec4 shadowUV;
#endif

void main(void)
{
	// Vertex location
	gl_Position = modelViewProj * vertex;

	// Texture coordinates
	uv = uv0;

#ifdef SHADOW_MAP
	// Shadow map tex coords
	vec4 worldPos = model * vertex;
	shadowUV = texViewProj * worldPos;
#endif

	// Vertex coords from eye position
	vec3 mvVertex = vec3(modelView * vertex);
	vVertex = mvVertex;

	// w component needs to be zero otherwise the eye distance is affecting
	// the normal
	// Needs to be normalized for some objects
	// Those that have scale, I'm guessing?
	vNormal = normalize(vec3(modelView * vec4(normal, 0.0)));

	// Eye direction from vertex, for half vector
	// If eye position is at (0, 0, 0), -mvVertex points
	// to eye position from vertex. Otherwise
	// direction to eye is: eyePosition - mvVertex
	vec3 mvDirToEye = -mvVertex;
    
	dirToEye = normalize(mvDirToEye);

	/// Lights
	int n_lights = int(nLights) < MAX_LIGHTS ? int(nLights) : MAX_LIGHTS;

	// Light direction from vertex
	for(int i = 0; i < n_lights; ++i)
	{
		lightPos[i] = vec3(mvLightPos[i]);

		spotlightDir[i] = spotDirection[i].xyz;
	}
}

