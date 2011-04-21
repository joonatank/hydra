// The Fragment Program

// TODO hasn't been tested with specular lighting
// works fine with white specular map and not defining specular colour in material (black spec)
// TODO add emissive materials
// TODO test point lights
// TODO test directional lights
// TODO add vertex colours, needs an uniform param wether they are enabled or not
//
// Spotlights tested with inner, outer and falloff
// Spotlights tested with changing position and orientation
// Single depth shadow map supported. As simple as possible.

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
uniform sampler2D shadowMap;

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

// Vertex colour
// Not supported yet, these need a whether or not the material has them or not
in vec4 vColour;

// Shadow map uvs, x,y are the coordinates on the texture
// z is the distance to light
in vec4 shadowUV;

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
	// from DirectX documentation the spotlight factor
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	float spotFactor = clamp((rho - outer_a)/(inner_a - outer_a), 0.0, 1.0);

	// FIXME this check needs to be rewriten for Ogre spotlights
//	if( spotFactor > spotlightParams.y )
	{
		spotFactor = pow(spotFactor, falloff);
		float att = spotFactor * attenuation;

		// Normalize interpolated direction to light
		vec3 normDirToLight = normalize(dirToLight);
		// Full strength if normal points directly at light
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

	// Projective shadows, and the shadow texture is a depth map
	// note the perspective division!
	vec3 tex_coords = shadowUV.xyz/shadowUV.w;
	// read depth value from shadow map
	float depth = texture(shadowMap, tex_coords.xy).r;
	float inShadow = (depth > tex_coords.z) ? 1.0 : 0.0;

	colour = inShadow*(diffuse + specular);

	FragmentColour = clamp(colour, 0.0, 1.0);

	// Use the diffuse texture's alpha value.
	FragmentColour.a = diffuseTexColour.a;
}

