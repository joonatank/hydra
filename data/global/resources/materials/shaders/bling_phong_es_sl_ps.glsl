// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Bling-Phong shading program for single light in eye space
// The Fragment Program

// FIXME there is a problem when using specular lighting without specular map
// works fine with white specular map and not defining specular colour in material (black spec)
// TODO test directional lights
// TODO add vertex colours, needs preprocessor define
//
// Spotlights tested with inner, outer and falloff
// Spotlights tested with changing position and orientation
// Single depth shadow map supported. As simple as possible.
// Shadows can be turned on/off with SHADOW_MAP define

/// Version 120 is the most recent supported by G71 (Quadro FX 5500)
/// If you need more recent features, please do create another shader
/// for more recent cards.
#version 140

// Material parameters
uniform vec4 surfaceDiffuse;
uniform vec4 surfaceSpecular;
uniform float shininess; // Shininess exponent for specular highlights

// Light parameters
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 spotlightParams;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularMap;

#ifdef SHADOW_MAP
uniform sampler2D shadowMap;
#endif

#ifdef SHADOW_MAP
// Wether or not this light casts shadows
uniform float lightCastsShadows;
#endif

in vec4 uv;

in vec3 vNormal;

// Space in which these are does not matter as all the transformation
// should be done in the vertex shader and they should be in the same
// space here be that object, eye or tangent.
// Direction to light
in vec3 dirToLight;
// from vertex to eye in eye space
in vec3 dirToEye;
// How much the light is to be attenuated, this includes both spotlight
// Should these be calculated in the pixel shader?
in float attenuation;

// Spotlight direction in eye space.
in vec3 spotlightDir;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
#ifdef SHADOW_MAP
in vec4 shadowUV;
#endif

out vec4 FragmentColour;

void main(void)
{
	// Base colour from diffuse texture
	vec4 diffuseTexColour = texture2D(diffuseTexture, uv.xy);

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
		float att = spotFactor * attenuation;

		// Normalize interpolated direction to light
		vec3 normDirToLight = normalize(dirToLight);
		// Full strength if normal points directly at light
		float lambertTerm = max(dot(normDirToLight, vNormal), 0.0);
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
			float HdotN = max(dot(half_v, vNormal), 0.0); 
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

