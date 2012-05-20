// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2012-04

#version 130

// Calculate the attenuation parameter
float calculate_attenuation(vec3 light_pos, vec4 attenuation)
{
	// check for zero range
	if(attenuation.x == 0.0)
	{ return 0.0; }

	float distSqr = dot(light_pos, light_pos);
	// Calculating everything as squared
	// this will fail for range = 0
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
	float distAtt = clamp(1.0 - invRadiusSqr * distSqr, 0.0, 1.0);

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
	// point and directional lights which creates incorrect shading in some
	// positions
	if( all(equal(spot_params, vec4(1.0, 0.0, 0.0, 1.0) )) )
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

float is_in_shadow(sampler2D shadow_map, vec4 shadow_uv,
		float inverse_shadowmap_size, float fixed_depth_bias,
		float gradient_clamp, float gradient_scale_bias)
{
	// Projective shadows, and the shadow texture is a depth map
	// note the perspective division!
	vec3 tex_coords = shadow_uv.xyz/shadow_uv.w;

	// read depth value from shadow map
	float centerdepth = texture(shadow_map, tex_coords.xy).x;

	// gradient calculation
	float pixeloffset = inverse_shadowmap_size;
	vec4 depths = vec4(
		texture(shadow_map, tex_coords.xy + vec2(-pixeloffset, 0)).x,
		texture(shadow_map, tex_coords.xy + vec2(+pixeloffset, 0)).x,
		texture(shadow_map, tex_coords.xy + vec2(0, -pixeloffset)).x,
		texture(shadow_map, tex_coords.xy + vec2(0, +pixeloffset)).x);

	vec2 differences = abs( depths.yw - depths.xz );
	float gradient = min(gradient_clamp, max(differences.x, differences.y));
	float gradientFactor = gradient * gradient_scale_bias;

	// visibility function
	float depthAdjust = gradientFactor + (fixed_depth_bias * centerdepth);
	float finalCenterDepth = centerdepth + depthAdjust;

#if USE_PCF
	// use depths from prev, calculate diff
	depths += depthAdjust;
	float final = (finalCenterDepth > tex_coords.z) ? 1.0 : 0.0;
	final += (depths.x > tex_coords.z) ? 1.0 : 0.0;
	final += (depths.y > tex_coords.z) ? 1.0 : 0.0;
	final += (depths.z > tex_coords.z) ? 1.0 : 0.0;
	final += (depths.w > tex_coords.z) ? 1.0 : 0.0;

	return 0.2 * final;
#else
	return (finalCenterDepth > tex_coords.z) ? 1.0 : 0.0;
#endif
}

/// @return the fragment colour (combined specular and diffuse)
vec4 bling_phong_lighting(vec3 dir_to_light, vec3 dir_to_eye, vec3 normal,
		vec4 surface_diffuse, vec4 surface_specular, float shininess,
		vec4 light_diffuse, vec4 light_specular, float attenuation)
{
	vec4 colour = vec4(0.0, 0.0, 0.0, 1.0);

	if(attenuation == 0.0)
	{ return colour; }

	// Full strength if normal points directly at light
	float lambertTerm = max(dot(dir_to_light, normal), 0.0);
	// Only calculate diffuse and specular if light reaches the fragment.
	if(lambertTerm > 0.0)
	{
		// Diffuse
		colour = attenuation * light_diffuse * surface_diffuse * lambertTerm;

		// Specular
		// Always calculate because checking at run time is more expensive

		// dir_to_eye shouldn't need to be normalized
		vec3 half_v = normalize(dir_to_eye + dir_to_light);

		// Specular strength, Blinn-Phong shading model
		float HdotN = max(dot(half_v, normal), 0.0); 
		// FIXME the speculars don't work correctly
		colour += attenuation * light_specular * surface_specular
			* pow(HdotN, shininess);
	}

	// Reset alpha to diffuse alpha
	colour.a = surface_diffuse.a;

	return colour;
}

