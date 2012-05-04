
#version 120

attribute vec4 vertex;
attribute vec3 normal; 
attribute vec4 uv0;

varying vec3 v_pos;
varying vec2 v_uv;
varying vec3 v_normal; 

uniform mat4 g_wvp;
// Model view matrix also known as world view
uniform mat4 g_mw;

void main()
{
	gl_Position = g_wvp * vertex;
	v_uv = uv0.xy;
	/// Normals and position to eye space
	//v_normal = (g_world * normal).xyz; 
	v_normal = normalize((g_mw * vec4(normal, 0.0)).xyz);
	v_pos = (g_mw * vertex).xyz;
}

