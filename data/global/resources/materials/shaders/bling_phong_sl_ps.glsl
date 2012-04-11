// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Bling-Phong shading program for single light
// The Fragment Program

// TODO test directional lights
// TODO specular lights are not correct
//
// Spotlights tested with inner, outer and falloff
// Spotlights tested with changing position and orientation
// Single depth shadow map supported. As simple as possible.

// Defines:
// These values must correspond to the ones in vertex shader.
// SHADOW_MAP : Shadows can be turned on/off.
// NORMAL_MAP : Normal mapping can be turned on/off.
// Pixel shader only defines.
// NEW_ATTENUATION : Turns on/off quadratic distance for attenuation
//	creates high lights for objects for which distance << light_max_distance


// Version parameter does not work correctly in Ogre, the main tree uses
// OpenGL 2. No harm about it though. Waiting for OpenGL 3+ rendering system.
// Well the version parameter is working fine though Ogre only supports OpenGL 2
// the shaders can be writen for OpenGL 3+ and compiled for that if the
// hardware supports them.
// We are using the new in/out syntax introduced in GLSL 1.4 so don't change
// the version number.
#version 140

// Material parameters
uniform float shininess; // Shininess exponent for specular highlights
uniform vec4 surfaceDiffuse;
uniform vec4 surfaceSpecular;

// Light parameters
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 spotlightParams;
uniform vec4 lightAttenuation;

#ifdef SHADOW_MAP
uniform float lightCastsShadows;
#endif

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularMap;
#ifdef NORMAL_MAP
uniform sampler2D normalMap;
#endif
#ifdef SHADOW_MAP
uniform sampler2D shadowMap;
// Some extra parameters for shadows
uniform float inverseShadowmapSize;
uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;
#endif


in vec4 uv;

// Used if no normal mapping
#ifndef NORMAL_MAP
in vec3 vNormal;
#endif

// Space in which these are does not matter as all the transformation
// should be done in the vertex shader and they should be in the same
// space here be that object, eye or tangent.

// From vertex to eye
in vec3 dirToEye; 
// Light position needed for attenuation calculation.
// Space where this is does not really matter because it's only used for calculating
// the distance from the fragment to be lit.
// Of course the position does not make any sense if it's not a position from the vertex.
// Attenuation in pixel shader otherwise large objects
// with few vertices are incorrectly lit
in vec4 lightPos;

// Spotlight direction, should be in the same space as the rest of the parameters,
in vec3 spotlightDir;

// Vertex for calculating light direction vector
// Calculating direction vector in Vertex shader screws up the accuracy for low poly models
// Try it using a 20 x 40 meter plane with four vertices.
in vec3 vVertex;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
#ifdef SHADOW_MAP
in vec4 shadowUV;
#endif

out vec4 FragmentColour;

// Declarations for external functions to be linked
float calculate_attenuation(vec3 light_pos, vec4 attenuation);
float calculate_spot(vec3 light_dir, vec3 spot_dir, vec4 spot_params);

float is_in_shadow(sampler2D shadow_map, vec4 shadow_uv,
		float inverse_shadowmap_size, float fixed_depth_bias,
		float gradient_clamp, float gradient_scale_bias);

vec4 bling_phong_lighting(vec3 dir_to_light, vec3 dir_to_eye, vec3 normal,
		vec4 surface_diffuse, vec4 surface_specular, float shininess,
		vec4 light_diffuse, vec4 light_specular, float attenuation);


void main(void)
{
#ifdef NORMAL_MAP
	vec4 normalTexColour = texture2D(normalMap, uv.xy);

	// Uncompress normal from normal map texture
	vec3 normal = normalize(normalTexColour.xyz * 2.0 - 1.0);
#else
	vec3 normal = vNormal;
#endif

	// Light vector needs to be calculated in fragment shader for low
	// poly objects like flat walls.
	// For directional lights lightPos.w is 0 others 1
	vec3 fragmentToLight = lightPos.xyz - (vVertex*lightPos.w);

	// Normalize interpolated direction to light
	vec3 normDirToLight = normalize(fragmentToLight);
	vec3 normSpotDir = normalize(spotlightDir);

	float spotFactor = calculate_spot(normDirToLight, normSpotDir, spotlightParams);

	float attenuation = 0.0;
	// Spotlight check
	if(spotFactor > 0.0)
	{ attenuation = spotFactor * calculate_attenuation(fragmentToLight, lightAttenuation); }

	vec4 colour = vec4(0.0, 0.0, 0.0, 1.0);

	if(attenuation > 0.0)
	{
		// Using both texture and surface colours for all objects cause minimal
		// (less than a precent) overhead so we definitely shouldn't change it.

		// Texture colours
		vec4 diffuseTexColour = texture2D(diffuseTexture, uv.xy);
		vec4 specularTexColour = texture2D(specularMap, uv.xy);

		// Combined surface colours
		vec4 diffuseColour = diffuseTexColour * surfaceDiffuse;
		vec4 specularColour = specularTexColour * surfaceSpecular;

		colour = bling_phong_lighting(normDirToLight, dirToEye, normal, diffuseColour, specularColour,
				shininess, lightDiffuse, lightSpecular, attenuation);
	}

	float inShadow = 1.0;
#ifdef SHADOW_MAP
	// Checking for fragment colour is more expensive so unless we expect
	// scenes with lots of completely black surfaces we shouldn't do it
	//if(lightCastsShadows > 0.0 && any(greaterThan(colour.xyz, vec3(0.0, 0.0, 0.0))) )
	if(lightCastsShadows > 0.0 && attenuation > 0.0)
	{
		inShadow = is_in_shadow(shadowMap, shadowUV, inverseShadowmapSize,
				fixedDepthBias, gradientClamp, gradientScaleBias);
	}
#endif

	// TODO this will distort the alpha channel
	FragmentColour = clamp(inShadow * colour, 0.0, 1.0);
}

