
#version 120

// Material parameters
uniform vec4 g_surface_diffuse;
uniform vec4 g_surface_specular;
uniform float g_surface_shininess;
uniform vec4 g_surface_ambient;
uniform vec4 g_surface_emissive;

// Textures
uniform sampler2D g_diffuse_map; 

varying vec3 v_pos;
varying vec2 v_uv;
varying vec3 v_normal;

void main()
{
	// @todo Probably should move things to screen space here
	// First world positions
	float spec = g_surface_specular.x + g_surface_specular.y
		+ g_surface_specular.z;
	spec = clamp(spec/3, 0, 1);
	gl_FragData[0] = vec4(v_pos, spec);
	// Second world normals
	float spec_power = g_surface_shininess;
	gl_FragData[1] = vec4(normalize(v_normal), spec_power);
	// Third diffuse colour
	vec4 amb_vec = g_surface_ambient + g_surface_emissive;
	float amb = (amb_vec.x + amb_vec.y + amb_vec.z)/3;
	float global_ambient = clamp(amb, 0, 1);
	vec4 tex_col = texture2D(g_diffuse_map, v_uv);	
	vec4 col = g_surface_diffuse * tex_col;
	gl_FragData[2] = vec4(col.xyz, global_ambient);
	// Fourth not needed
	// FSout.TexCoord = vec3(FSin.TexCoord, 0.0);	
	/*
	gl_FragData[0] = vec4(1.0, 0.0, 1.0, 1.0);
	gl_FragData[1] = vec4(0.0, 1.0, 1.0, 1.0);
	gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
	*/
}


