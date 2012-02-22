// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Simple depth shadow caster vertex shader

// Defines:
// USE_SCENE_RANGE : use depth range parameters from Ogre

// This really don't need to be GLSL 1.4
#version 140

uniform mat4 modelViewProj;	// Model view projection matrix
uniform vec4 texelOffsets;
#ifdef USE_SCENE_RANGE
uniform vec4 sceneRange;
#endif

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
#ifdef USE_SCENE_RANGE
	depth.x = (gl_Position.z - sceneRange.x) * sceneRange.w;
	// TODO should this do something with gl_Position.w?
	depth.y = gl_Position.w;
#else
	depth.x = gl_Position.z;
	depth.y = gl_Position.w;
#endif
}

