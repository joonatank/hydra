// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Tangent space vertex program for single light

// Defines:
// These values must correspond to the ones in pixel shader.
// SHADOW_MAP : Shadows can be turned on/off.
// NORMAL_MAP : Normal mapping can be turned on/off.

#version 140

uniform mat4 modelView;
uniform mat4 modelViewProj;
uniform mat4 normalMatrix;
// Light position in eye space
uniform vec4 mvLightPos;
// Renaming light direction to spotlight direction because
// it's only usefull for spotlights
uniform vec4 spotDirection;

#ifdef SHADOW_MAP
// Shadow transformation matrix
uniform mat4 model;
// Shadow texture transform
uniform mat4 texViewProj;
#endif

// The order of these is significant
// my guess is that uv0 can not appear after tangent and binormal (uv6 and uv7)
in vec4 vertex;
in vec3 normal;
in vec4 uv0;
in vec3 tangent;
// Does not give the expected results i.e. [0 -1 0], gives [0 0 0]
//in vec3 binormal;

out vec4 uv;

#ifndef NORMAL_MAP
out vec3 vNormal;
#endif

// from vertex to light in tangent space
//out vec3 dirToLight;
// from vertex to eye in tangent space, used for specular highlights
out vec3 dirToEye;
// Tangent space light position for attenuation and spotlight
// For low poly object this needs to pass through to get the correct
// interpolated values for fragment.
out vec4 lightPos;
// Light to vertex in tangent space
// Spotlight direction vector in tangent space
out vec3 spotlightDir;
// Vertex in tangent space for calculating light direction
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

	// Shadow map tex coords
#ifdef SHADOW_MAP
	vec4 worldPos = model * vertex;
	shadowUV = texViewProj * worldPos;
#endif

	// Tangent space vectors (TBN) in eye space
	// normalMatrix = gl_NormalMatrix = inverse transpose model view
	// except it's 4x4 and the vectors need to be normalized after transform
	vec3 mvTangent = normalize((normalMatrix * vec4(tangent, 1.0)).xyz);
	vec3 mvNormal = normalize((normalMatrix * vec4(normal, 1.0)).xyz);
	vec3 mvBinormal = cross(mvTangent, mvNormal);

#ifndef NORMAL_MAP
	vNormal.x = dot(mvNormal, mvTangent);
	vNormal.y = dot(mvNormal, mvBinormal);
	vNormal.z = dot(mvNormal, mvNormal);
#endif

	// Vertex coords from eye position
	vec3 mvVertex = vec3(modelView * vertex);

	vVertex.x = dot(mvVertex, mvTangent);
	vVertex.y = dot(mvVertex, mvBinormal);
	vVertex.z = dot(mvVertex, mvNormal);

	// Eye direction from vertex, for half vector
	// If eye position is at (0, 0, 0), -mvVertex points
	// to eye position from vertex. Otherwise
	// direction to eye is: eyePosition - mvVertex
	vec3 mvDirToEye = -mvVertex;
	vec3 tbnDirToEye;
	// TBN x lightDir does not give us the right answer
	// the light is moving wildly and rotate camera by 180 around y
	// gives us a black picture
	tbnDirToEye.x = dot(mvDirToEye, mvTangent);
	tbnDirToEye.y = dot(mvDirToEye, mvBinormal);
	tbnDirToEye.z = dot(mvDirToEye, mvNormal);
    
	dirToEye = normalize(tbnDirToEye);

	// Light direction from vertex
	// lightPos.w is for directional lights, they have w = 0
	vec3 tbnLightPos = mvLightPos.xyz;
	lightPos.x = dot(tbnLightPos, mvTangent);
	lightPos.y = dot(tbnLightPos, mvBinormal);
	lightPos.z = dot(tbnLightPos, mvNormal);
	// Copy w so that directional lights are handleded correctly
	lightPos.w = mvLightPos.w;
	
	// lightDirection is in eye space already and it's direction vector
	// not a position
	vec3 mvSpotlightDir = spotDirection.xyz;
	spotlightDir.x = dot(mvSpotlightDir, mvTangent);
	spotlightDir.y = dot(mvSpotlightDir, mvBinormal);
	spotlightDir.z = dot(mvSpotlightDir, mvNormal);
	spotlightDir = normalize(spotlightDir);
}

