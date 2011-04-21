// Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
// Savant Simulators
// 2011-04
//
// Simple depth shadow caster fragment shader

// This really don't need to be GLSL 1.4
#version 140

in vec2 depth;

out vec4 FragmentColour;

void main(void)
{
    // Use some bias to avoid precision issue
    float l_Depth = depth.x / depth.y;

    // Write the depth value to the depth map
    FragmentColour = vec4(l_Depth, l_Depth, l_Depth, 1.0);
}

