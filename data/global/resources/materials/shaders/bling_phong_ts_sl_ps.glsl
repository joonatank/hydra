// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Bling-Phong shading program for single light
// The Fragment Program

// TODO test directional lights
// TODO add vertex colours, needs an define if the user wants to compile them
// or not
// TODO specular lights are not correct
//
// Spotlights tested with inner, outer and falloff
// Spotlights tested with changing position and orientation
// Single depth shadow map supported. As simple as possible.

/// Version 120 is the most recent supported by G71 (Quadro FX 5500)
/// If you need more recent features, please do create another shader
/// for more recent cards.
// Version parameter does not work correctly in Ogre, the main tree uses
// OpenGL 2. No harm about it though. Waiting for OpenGL 3+ rendering system.
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
uniform sampler2D normalMap;
#ifdef SHADOW_MAP
uniform sampler2D shadowMap;
#endif

in vec4 uv;

// Space in which these are does not matter as all the transformation
// should be done in the vertex shader and they should be in the same
// space here be that object, eye or tangent.
// Direction to light
in vec3 dirToLight;
// from vertex to eye in tangent space
in vec3 dirToEye; 
// Eye space light position needed for attenuation calculation
// Attenuation in pixel shader otherwise large objects
// with few vertices are incorrectly lit
in vec3 mvLightPos;

// Spotlight direction, should be in the same space as the rest of the parameters,
// especially dirToLight.  Tangent space is tested, but eye space should also work.
in vec3 spotlightDir;

// Vertex colour
// Not supported yet, these need a switch for the material
in vec4 vColour;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
#ifdef SHADOW_MAP
in vec4 shadowUV;
#endif

out vec4 FragmentColour;

// Calculate the attenuation parameter
// Compute attenuation, in eye space
float attenuate(vec3 lightPos, vec4 attenuation)
{
	float distSqr = dot(lightPos, lightPos);
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

	float d = length(lightPos);
	float att =  distAtt/( attenuation.y +
		(attenuation.z * d) +
		(attenuation.w * d*d) );

	return clamp(att, 0.0, 1.0);
}

void main(void)
{
	// Base colour from diffuse texture
	vec4 diffuseTexColour = texture2D(diffuseTexture, uv.xy);
	vec4 normalTexColour = texture2D(normalMap, uv.xy);

	// Uncompress normal from normal map texture
	vec3 normal = normalize(normalTexColour.xyz * 2.0 - 1.0);

	vec4 colour = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

	// Spotlight check
	// factor in spotlight angle
	float rho = dot(normalize(spotlightDir), normalize(dirToLight));
	// Ogre gives us spotlight params as a
	// vec4(cos(inner_angle/2), cos(outer_angle/2), falloff, 1)
	float inner_a = spotlightParams.x;
	float outer_a = spotlightParams.y;
	float falloff = spotlightParams.z;
	// from DirectX documentation the spotlight factor
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	float spotFactor = clamp((rho - outer_a)/(inner_a - outer_a), 0.0, 1.0);

	if( spotFactor > 0 )
	{
		spotFactor = pow(spotFactor, falloff);

		// Calculate the attenuation
		float att = spotFactor *
			attenuate(mvLightPos, lightAttenuation);

		// Normalize interpolated direction to light
		vec3 normDirToLight = normalize(dirToLight);
		// Full strength if normal points directly at light
		float lambertTerm = max(dot(normDirToLight, normal), 0.0);
		// Only calculate diffuse and specular if light reaches the fragment.
		if (lambertTerm > 0.0)
		{
			// Diffuse
			vec4 diffuseColour = diffuseTexColour * surfaceDiffuse;
			diffuse = att * lightDiffuse * diffuseColour
				* lambertTerm;

			// Specular
			// Colour of specular reflection
			vec4 specularColour = texture2D(specularMap, uv.xy); 
			specularColour *= surfaceSpecular;

			vec3 half_v = normalize(dirToEye + dirToLight);

			// Specular strength, Blinn-Phong shading model
			float HdotN = max(dot(normalize(half_v), normal), 0.0); 
			// FIXME the speculars don't work correctly
			specular = att * lightSpecular * specularColour
				* pow(HdotN, shininess);
		}
	}

	float inShadow = 1.0;
#ifdef SHADOW_MAP
	if(lightCastsShadows > 0.0)
	{
		// Projective shadows, and the shadow texture is a depth map
		// note the perspective division!
		vec3 tex_coords = shadowUV.xyz/shadowUV.w;
		// read depth value from shadow map
		float depth = texture(shadowMap, tex_coords.xy).r;
		inShadow = (depth > tex_coords.z) ? 1.0 : 0.0;
	}
#endif

	colour = inShadow*(diffuse + specular);

	FragmentColour = clamp(colour, 0.0, 1.0);

	// Use the diffuse texture's alpha value.
	FragmentColour.a = diffuseTexColour.a;
}

