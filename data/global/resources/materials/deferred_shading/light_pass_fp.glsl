// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2012-05
//
// Deferred shading light pass fragment program
// Applies bling-phong shading to whole scene.

#version 120

uniform vec4 g_viewport_size;

uniform vec4 g_light_diffuse;
uniform vec4 g_light_specular;
uniform vec4 g_light_attenuation;
uniform vec4 g_spotlight_params;
uniform vec4 g_spot_direction;
// In world space
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
uniform mat4 g_tex_vp;
uniform float g_inverse_shadowmap_size;
#ifdef USE_SCENE_RANGE
uniform vec4 g_shadow_scene_range;
#endif
// Shadow map
uniform sampler2D g_shadow_map;
#endif

// Custom parameters
uniform vec4 g_camera_position;

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
}

void main(void)
{
	vec2 uv = calc_tex_coord();
	// In world space
	vec3 vertex_pos = texture2D(g_position_buffer, uv).xyz;
	vec3 normal = normalize(texture2D(g_normal_buffer, uv).xyz);
	vec4 surface_diffuse = vec4(texture2D(g_colour_buffer, uv).xyz, 1.0);
	float spec = texture2D(g_position_buffer, uv).w;
	float power = texture2D(g_normal_buffer, uv).w;
	float ga = texture2D(g_colour_buffer, uv).w;

	vec4 surface_specular = vec4(spec, spec, spec, 1.0);

	// missing
	// global ambient

	vec3 light_to_vertex = g_light_position.xyz - vertex_pos*g_light_position.w;
	vec3 dir_to_light = normalize(light_to_vertex);

	// We have vertex position in world so we need camera in world as well
	vec3 dir_to_eye = normalize(g_camera_position.xyz - vertex_pos);

	float attenuation = calculate_attenuation(light_to_vertex, g_light_attenuation);

	attenuation *= calculate_spot(dir_to_light, g_spot_direction.xyz, g_spotlight_params);
	float in_shadow = 1.0;
#ifdef SHADOW_MAP
	// Checking for fragment colour is more expensive so unless we expect
	// scenes with lots of completely black surfaces we shouldn't do it
	//if(lightCastsShadows > 0.0 && any(greaterThan(colour.xyz, vec3(0.0, 0.0, 0.0))) )
	if(g_light_casts_shadows > 0.0 && attenuation > 0.0)
	{
		// Vertex is already stored in world space
		vec4 vertex_world = vec4(vertex_pos, 1.0);
		// @todo rename shadow_uv to something more descriptive
		vec4 shadow_uv = g_tex_vp * vertex_world;
#ifdef USE_SCENE_RANGE
		// make linear
		shadow_uv.z = (shadow_uv.z - g_shadow_scene_range.x) * g_shadow_scene_range.w;
#endif

		in_shadow = is_in_shadow(g_shadow_map, shadow_uv,
				g_inverse_shadowmap_size, 0, 0, 0);
	}
#endif


	vec4 colour = vec4(0.0, 0.0, 0.0, 1.0);
	if(in_shadow > 0.0)
	{
		colour = in_shadow * bling_phong_lighting(dir_to_light, dir_to_eye, normal,
			surface_diffuse, surface_specular, power,
			g_light_diffuse, g_light_specular, attenuation);
	}

	gl_FragColor = colour;
}

