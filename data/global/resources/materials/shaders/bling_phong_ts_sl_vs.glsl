// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Bling-Phong (tangent-space) shading program for single light
// The Vertex Program

#version 140

uniform mat4 modelView;
uniform mat4 modelViewProj;
uniform mat4 normalMatrix;
// Light position in eye space
uniform vec4 lightPos;
uniform vec4 lightAttenuation;
uniform vec4 spotDirection;

uniform mat4 model;
// Shadow texture transform
uniform mat4 texViewProj;

// The order of these is significant
// my guess is that uv0 can not appear after tangent and binormal (uv6 and uv7)
in vec4 vertex;
in vec3 normal;
in vec4 uv0;
in vec3 tangent;
// Does not give the expected results i.e. [0 -1 0], gives [0 0 0]
//in vec3 binormal;

out vec4 uv;

// from vertex to light in tangent space
out vec3 dirToLight;
// from vertex to eye in tangent space, used for specular highlights
out vec3 dirToEye;
// Attenuation parameter
out float attenuation;
// Light to vertex in tangent space
//out vec4 vertexToLight;
// Spotlight direction vector, should be vec4(1,0,0,1) for anything else
// in tangent space
out vec3 spotlightDir;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
out vec4 shadowUV;

void main(void)
{
	// Vertex location
	gl_Position = modelViewProj * vertex;

	// Texture coordinates
	uv = uv0;

	// Shadow map tex coords
	vec4 worldPos = model * vertex;
	shadowUV = texViewProj * worldPos;

	// Tangent space vectors (TBN) in eye space
	// normalMatrix = gl_NormalMatrix = inverse transpose model view
	// except it's 4x4 and the vectors need to be normalized after transform
	vec3 mvTangent = normalize((normalMatrix * vec4(tangent, 0.0)).xyz);
	vec3 mvNormal = normalize((normalMatrix * vec4(normal, 0.0)).xyz);
	vec3 mvBinormal = cross(mvTangent, mvNormal);

	// Vertex coords from eye position
	vec3 mvVertex = vec3(modelView * vertex);

	// Eye direction from vertex, for half vector
	// If eye position is at (0, 0, 0), -mvVertex points
	// to eye position from vertex. Otherwise
	// direction to eye is: eyePosition - mvVertex
	vec3 mvDirToEye = - mvVertex;
	vec3 tbnDirToEye;
	// TBN x lightDir does not give us the right answer
	// the light is moving wildly and rotate camera by 180 around y
	// gives us a black picture
	tbnDirToEye.x = dot(mvDirToEye, mvTangent);
	tbnDirToEye.y = dot(mvDirToEye, mvBinormal);
	tbnDirToEye.z = dot(mvDirToEye, mvNormal);
    
	// Light direction from vertex
	// lightPos.w is for directional lights, they have w = 0
	vec3 mvLightPos = vec3(lightPos) - mvVertex*lightPos.w;
	// normalizing the direction does not make a difference
	vec3 mvLightDir = normalize(mvLightPos);

	// Light in tangent space
	dirToLight.x = dot(mvLightDir, mvTangent);
	dirToLight.y = dot(mvLightDir, mvBinormal);
	dirToLight.z = dot(mvLightDir, mvNormal);
	
	vec3 mvSpotlightDir = -spotDirection.xyz;
	spotlightDir.x = dot(mvSpotlightDir, mvTangent);
	spotlightDir.y = dot(mvSpotlightDir, mvBinormal);
	spotlightDir.z = dot(mvSpotlightDir, mvNormal);

	dirToEye = tbnDirToEye;

	// Compute attenuation, in eye space
	// TODO add range parameter
	float d = length(mvLightPos);
	attenuation =  1.0/( lightAttenuation.y +
		(lightAttenuation.z * d) +
		(lightAttenuation.w * d*d) );
}

