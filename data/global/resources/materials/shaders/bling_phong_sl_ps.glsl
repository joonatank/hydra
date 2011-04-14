// The Fragment Program

// TODO hasn't been tested with specular lighting
// works fine with white specular map and not defining specular colour in material (black spec)
// TODO add emissive materials
// TODO test point lights
// TODO test directional lights
// TODO add shadow textures
//
// Spotlights tested with inner, outer and falloff
// Spotlights tested with changing position and orientation

/// Version 120 is the most recent supported by G71 (Quadro FX 5500)
/// If you need more recent features, please do create another shader
/// for more recent cards.
#version 140

uniform float shininess; // Shininess exponent for specular highlights
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 spotlightParams;

uniform sampler2D diffuseTexture;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

in vec4 uv;

in vec3 oNormal;
// Space in which these are does not matter as all the transformation
// should be done in the vertex shader and they should be in the same
// space here be that object, eye or tangent.
// Direction to light
in vec3 dirToLight;
// Half vector
in vec3 halfVector; 
// How much the light is to be attenuated, this includes both spotlight
// Should these be calculated in the pixel shader?
in float attenuation;

// Spotlight direction, should be in the same space as the rest of the parameters,
// especially dirToLight.  Tangent space is tested, but eye space should also work.
in vec3 spotlightDir;

out vec4 FragmentColour;

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
	// factor (from DX documentation)
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	float spotFactor = clamp((rho - outer_a)/(inner_a - outer_a), 0.0, 1.0);

//	if( spotFactor > spotlightParams.y )
	{
		spotFactor = pow(spotFactor, falloff);
		float att = spotFactor * attenuation;

		// Normalize interpolated direction to light
		vec3 normDirToLight = normalize(dirToLight);
		// Full strength if normal points directly at light
		// FIXME for some reason the lambertTerm is zero with
		// flat normal map. -> we get black shading
		float lambertTerm = max(dot(normDirToLight, normal), 0.0);
		// Only calculate diffuse and specular if light reaches the fragment.
		if (lambertTerm > 0.0)
		{
			// Diffuse
			diffuse = att * lightDiffuse * diffuseTexColour
				* lambertTerm;

			// Specular
			// Colour of specular reflection
			vec4 specularColour = texture2D(specularMap, uv.xy); 
			// Specular strength, Blinn-Phong shading model
			float specularModifier =
				max(dot(normal, normalize(halfVector)), 0.0); 
			specular = att * lightSpecular * specularColour
				* pow(specularModifier, shininess);
		}
	}
	// Sum of all lights
	colour += (diffuse + specular);

	FragmentColour = clamp(colour, 0.0, 1.0);

	// Use the diffuse texture's alpha value.
	FragmentColour.a = diffuseTexColour.a;
}

