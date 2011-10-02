// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Bling-Phong shading program for multiple lights
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

uniform vec4 lightAmbient;
uniform vec4 surfaceAmbient;
uniform vec4 surfaceEmissive;

// Light parameters
uniform vec4 lightDiffuse[MAX_LIGHTS];
uniform vec4 lightSpecular[MAX_LIGHTS];
uniform vec4 spotlightParams[MAX_LIGHTS];
uniform vec4 lightAttenuation[MAX_LIGHTS];

#ifdef SHADOW_MAP
uniform float lightCastsShadows0;
uniform float lightCastsShadows1;
uniform float lightCastsShadows2;
uniform float lightCastsShadows3;
uniform float lightCastsShadows4;
uniform float lightCastsShadows5;
uniform float lightCastsShadows6;
uniform float lightCastsShadows7;
#endif

uniform float nLights;

// Textures
uniform sampler2D diffuseTexture;
#ifdef SPECULAR_MAP
uniform sampler2D specularMap;
#endif
#ifdef AMBIENT_OCCLUSION
uniform sampler2D aoMap;
#endif
#ifdef NORMAL_MAP
uniform sampler2D normalMap;
#endif
#ifdef SHADOW_MAP
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform sampler2D shadowMap3;
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
in vec3 lightPos[MAX_LIGHTS];

// Spotlight direction, should be in the same space as the rest of the parameters,
in vec3 spotlightDir[MAX_LIGHTS];

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

// Calculate the attenuation parameter
float calculate_attenuation(vec3 light_pos, vec4 attenuation)
{
	float distSqr = dot(light_pos, light_pos);
	// Calculating everything as squared
	// TODO this will fail for range = 0
	float invRadius = 1.0/attenuation.x;
	float invRadiusSqr = invRadius*invRadius;
	float radiusSqr = attenuation.x*attenuation.x;
	
	// distance attenuation
	// larger than 1 if the distance < radius, 1 for distance = radius
	// we want the distance attenuation to be
	// distAtt > 1 when distance << radius
	// distAtt = [0, 1] when distance < radius
	// distAtt = 0 when distance == radius
	// this is because for large distances the quadratic term in attenuation
	// will dominate the light intensity so we compensate it with the
	// difference in radius and distance
#ifdef NEW_ATTENUATION
	// Taking a square root would help with objects that are close
	// but would ruin this function if objects are far away
	float distAtt = radiusSqr/distSqr -1;
#else
	// Old function creates values in [0, 1]
	float distAtt = clamp(1.0 - invRadiusSqr * distSqr, 0.0, 1.0);
#endif

	float d = length(light_pos);
	float att =  distAtt/( attenuation.y +
		(attenuation.z * d) +
		(attenuation.w * d*d) );

	return clamp(att, 0.0, 1.0);
}

// @param light_dir normalized direction vector from vertex to light
// @param spot_dir normalized spotlight direction vector (the direction where the spot is at full strength)
// @param spot_params spotlight parameters as provided by Ogre
float calculate_spot(vec3 light_dir, vec3 spot_dir, vec4 spot_params)
{
	// Needs checking because otherwise would calculate the spot factor for
	// point and directional lights which creates incorrect shading in some positions
	if( spot_params == vec4(1.0, 0.0, 0.0, 1.0) )
	{ return 1.0; }

	// angle between surface vector and a spotlight direction vector
	float rho = dot(spot_dir, -light_dir);
	// Ogre gives us spotlight params as a
	// vec4(cos(inner_angle/2), cos(outer_angle/2), falloff, 1)
	float inner_a = spot_params.x;
	float outer_a = spot_params.y;
	float falloff = spot_params.z;
	// from DirectX documentation the spotlight factor
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	float factor = clamp((rho - outer_a)/(inner_a - outer_a), 0.0, 1.0);
	

	if(factor > 0.0)
	{
		factor = pow(factor, falloff);
	}
	return factor;
}

void main(void)
{
#ifdef NORMAL_MAP
	vec4 normalTexColour = texture2D(normalMap, uv.xy);

	// Uncompress normal from normal map texture
	vec3 normal = normalize(normalTexColour.xyz * 2.0 - 1.0);
#else
	vec3 normal = vNormal;
#endif

	/// Ambient
#ifdef AMBIENT_OCCLUSION
	vec4 aoColour = texture2D(aoMap, uv.xy);
	vec4 ambient = surfaceAmbient*aoColour*lightAmbient;
#else
	vec4 ambient = surfaceAmbient*lightAmbient;
#endif
	vec4 colour = ambient + surfaceEmissive;

	// Calculate surface values from texture and surface param
	// Outside the light iteration

	// Diffuse
	vec4 diffuseTexColour = texture2D(diffuseTexture, uv.xy);
	vec4 diffuseColour = diffuseTexColour * surfaceDiffuse;
	// Colour of specular reflection
#ifdef SPECULAR_MAP
	vec4 specularColour = texture2D(specularMap, uv.xy) * surfaceSpecular;
#else
	vec4 specularColour = surfaceSpecular;
#endif

	/// cast shadows
	float castsShadows[MAX_LIGHTS] = float[](lightCastsShadows0, lightCastsShadows1, lightCastsShadows2,
			lightCastsShadows3, lightCastsShadows4, lightCastsShadows5,
			lightCastsShadows6, lightCastsShadows7);
	int shadow_map_index = 0;

	int n_lights = int(nLights) < MAX_LIGHTS ? int(nLights) : MAX_LIGHTS;
	for(int i = 0; i < n_lights; ++i)
	{
		vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
		vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

		// Light vector needs to be calculated in fragment shader for low
		// poly objects like flat walls.
		vec3 fragmentToLight = lightPos[i] - vVertex;

		// Normalize interpolated direction to light
		vec3 normDirToLight = normalize(fragmentToLight);
		vec3 normSpotDir = normalize(spotlightDir[i]);

		float spotFactor = calculate_spot(normDirToLight, normSpotDir, spotlightParams[i]);

		// TODO move attenuation calculation here and add checking for zero to skip
		// the light calculation
		float attenuation = 0.0;
		// Spotlight check
		if(spotFactor > 0.0)
		{ attenuation = calculate_attenuation(fragmentToLight, lightAttenuation[i]); }

		float att = spotFactor * attenuation;
		if(att > 0.0)
		{
			// Full strength if normal points directly at light
			float lambertTerm = max(dot(normDirToLight, normal), 0.0);
			// Only calculate diffuse and specular if light reaches the fragment.
			if (lambertTerm > 0.0)
			{
				// Diffuse
				diffuse = att * lightDiffuse[i] * diffuseColour
					* lambertTerm;

				// Specular

				// Shouldn't need to be normalized but lets try it for now
				//vec3 normDirToEye = normalize(dirToEye);
				vec3 half_v = normalize(dirToEye + normDirToLight);

				// Specular strength, Blinn-Phong shading model
				float HdotN = max(dot(half_v, normal), 0.0); 
				// FIXME the speculars don't work correctly
				specular = att * lightSpecular[i] * specularColour
					* pow(HdotN, shininess);
			}
		}

		float inShadow = 1.0;
#ifdef SHADOW_MAP
		// Three shadow maps supported
		if(castsShadows[i] > 0.0 && att > 0.0 && shadow_map_index < 3)
		{
			// Projective shadows, and the shadow texture is a depth map
			// note the perspective division!
			vec3 tex_coords = shadowUV.xyz/shadowUV.w;
			// read depth value from shadow map
			float depth = 0;
			if(shadow_map_index == 0)
				depth = texture(shadowMap1, tex_coords.xy).r;
			else if(shadow_map_index == 1)
				depth = texture(shadowMap2, tex_coords.xy).r;
			else if(shadow_map_index == 2)
				depth = texture(shadowMap3, tex_coords.xy).r;
			inShadow = (depth > tex_coords.z) ? 1.0 : 0.0;
			shadow_map_index++;
		}
#endif
		colour += inShadow*(diffuse + specular);
	}	// end iterate per light

	// Combine diffuse alphas
	float alpha = diffuseTexColour.a*surfaceDiffuse.a;
	colour.a = alpha;

	// test how many lights
	FragmentColour = clamp(colour, 0.0, 1.0);
}

