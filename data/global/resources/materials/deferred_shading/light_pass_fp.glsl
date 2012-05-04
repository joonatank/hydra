
#version 120

uniform vec4 g_viewport_size;

uniform vec4 g_light_diffuse;
uniform vec4 g_light_specular;
// In eye space
uniform vec4 g_light_position;

// G-buffer
// Positions
uniform sampler2D g_position_buffer;
// Normals
uniform sampler2D g_normal_buffer;
// Diffuse colour
uniform sampler2D g_colour_buffer;


#ifdef SHADOW_MAP
// Shadow texture transform
uniform float g_light_casts_shadows;
uniform mat4 g_inv_view;
uniform mat4 g_tex_vp;
uniform float g_inverse_shadowmap_size;
// Shadow map
uniform sampler2D g_shadow_map;
#endif

//out vec4 FragmentColour;

/// External functions
float calculate_attenuation(vec3 light_pos, vec4 attenuation);
float calculate_spot(vec3 light_dir, vec3 spot_dir, vec4 spot_params);

float is_in_shadow(sampler2D shadow_map, vec4 shadow_uv,
		float inverse_shadowmap_size, float fixed_depth_bias,
		float gradient_clamp, float gradient_scale_bias);

vec4 bling_phong_lighting(vec3 dir_to_light, vec3 dir_to_eye, vec3 normal,
		vec4 surface_diffuse, vec4 surface_specular, float shininess,
		vec4 light_diffuse, vec4 light_specular, float attenuation);

vec2 calc_tex_coord()
{
	// Flip the texture coords, seems like a problem in the Float texture
	vec2 uv;
	uv.x = gl_FragCoord.x / g_viewport_size.x;
	uv.y = (1 -gl_FragCoord.y )/ g_viewport_size.y;
	return uv;
//	return gl_FragCoord.xy / g_viewport_size.xy;
}

/*
vec4 point_light(void)
{
   	 vec2 TexCoord = CalcTexCoord();
   	 vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
   	 vec3 Color = texture(gColorMap, TexCoord).xyz;
   	 vec3 Normal = texture(gNormalMap, TexCoord).xyz;
   	 Normal = normalize(Normal);

	 return vec4(Color, 1.0) * CalcPointLight(WorldPos, Normal);
}
*/

void main(void)
{
	vec2 uv = calc_tex_coord();
	// In eye space
	vec3 vertex_pos = texture2D(g_position_buffer, uv).xyz;
	vec3 normal = normalize(texture2D(g_normal_buffer, uv).xyz);
	vec4 surface_diffuse = vec4(texture2D(g_colour_buffer, uv).xyz, 1.0);
	float spec = texture2D(g_position_buffer, uv).w;
	float power = texture2D(g_normal_buffer, uv).w;
	float ga = texture2D(g_colour_buffer, uv).w;

	// missing
	// shininess or power
	// spec
	// global ambient
	// Remove the not so good specular highlights
	spec = 0.0;
	vec4 surface_specular = vec4(spec, spec, spec, 1.0);

	vec3 dir_to_light = g_light_position.xyz - vertex_pos*g_light_position.w;
	dir_to_light = normalize(dir_to_light);
	// Some testing
	// dir_to_light = normal;

	// Needs to be calculated from the g-buffer positions
	vec3 dir_to_eye = normalize(-vertex_pos);

	// @todo add real calculations
	float attenuation = 1.0;

	float in_shadow = 1.0;
#ifdef SHADOW_MAP
	// Checking for fragment colour is more expensive so unless we expect
	// scenes with lots of completely black surfaces we shouldn't do it
	//if(lightCastsShadows > 0.0 && any(greaterThan(colour.xyz, vec3(0.0, 0.0, 0.0))) )
	// Shadow map tex coords
	// We need to transform our vertex from eye space to world space
	// It's either inverse world view or inverse view
	// one of them transforms from view (eye) to model space other one
	// to world space
	// should be of course inverse view because the order of spaces is
	// model - world - view (naturally)
	// but of course we can't do this here because the matrix (inverse view)
	// is specific to our quad object... not the original object.
	// Does this matter?
	// The current view matrix is not the original view matrix because we don't
	// generally need it here.
	if(g_light_casts_shadows > 0.0 && attenuation > 0.0)
	{
		vec4 vertex_world = g_inv_view * vec4(vertex_pos, 1.0);
		vec4 shadow_uv = g_tex_vp * vertex_world;

		in_shadow = is_in_shadow(g_shadow_map, shadow_uv,
				g_inverse_shadowmap_size, 0, 0, 0);
	}
#endif


	vec4 colour = vec4(0.0, 0.0, 0.0, 1.0);
	if(in_shadow > 0.0)
	{
		colour = in_shadow * bling_phong_lighting(dir_to_light, dir_to_eye, normal,
			surface_diffuse, surface_specular, power,
			//vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), 1.0);
			g_light_diffuse, g_light_specular, attenuation);
	}

	gl_FragColor = colour;
}

