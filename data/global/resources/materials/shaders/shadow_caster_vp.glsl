// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Simple depth shadow caster vertex shader

// This really don't need to be GLSL 1.4
#version 140

uniform mat4 modelViewProj;	// Model view projection matrix
uniform vec4 texelOffsets;

// Define inputs from application.
in vec4 vertex;       // Vertex in object-space

out vec2 depth;

void main(void)
{
	// Transform vertex position into homogenous screen-space.
	gl_Position = modelViewProj * vertex;
    
	// fix pixel / texel alignment
	gl_Position.xy += texelOffsets.zw * gl_Position.w;

	// Store depth
	depth.x = gl_Position.z;
	depth.y = gl_Position.w;
}

