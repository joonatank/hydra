// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2012-05
//
// Deferred shading geometric pass vertex program

#version 120

attribute vec4 vertex;
attribute vec3 normal; 
attribute vec4 uv0;

varying vec3 v_pos;
varying vec2 v_uv;
varying vec3 v_normal; 

uniform mat4 g_wvp;
uniform mat4 g_world;

void main()
{
	gl_Position = g_wvp * vertex;
	v_uv = uv0.xy;
	/// Normals and position to world space
	v_normal = normalize((g_world * vec4(normal, 0.0)).xyz);
	v_pos = (g_world * vertex).xyz;
}

