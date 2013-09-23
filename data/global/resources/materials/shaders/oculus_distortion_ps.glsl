/// Pixel Shader
///
/// Distortion shader for Oculus Rift

#version 140

uniform sampler2D tex;
// What is SamplerState in GLSL?
//SamplerState Linear : register(s0);
uniform vec3 LensCenter;
uniform vec3 ScreenCenter;
// scale [out scale, in scale] where both out and in scale are vec2
uniform vec4 g_scale;
uniform vec4 HmdWarpParam;

// Not needed in the shader and GLSL has no predefined variables
//in vec4 oPosition; // : SV_Position;
//in vec4 oColor : COLOR;
in vec4 uv; // : TEXCOORD0;

out vec4 FragmentColour;

// Scales input texture coordinates for distortion.
vec2 HmdWarp(vec2 in01)
{
	vec2 scale_in = g_scale.zw;
	vec2 scale_out = g_scale.xy;
	vec2 theta = (in01 - LensCenter.xy) * scale_in; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 rvector= theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
			HmdWarpParam.z * rSq * rSq +
			HmdWarpParam.w * rSq * rSq * rSq);
	return LensCenter.xy + scale_out * rvector;
}

void main(void)
{
	// original value [0.25, 0.5] which has halved x because the render texture
	// has both left and right images
	vec2 offset = vec2(0.5, 0.5); //vec2(0.25,0.5);
	vec2 tc = HmdWarp(uv.xy);
	vec2 tc_min = ScreenCenter.xy-offset;
	vec2 tc_max = ScreenCenter.xy+offset;
	// clamp tc between min and max values if it's clampped
	// we are going off the screen so clear the pixel to black	
	// i.e. clamp(tc, min, max) != tc -> black
	if (any(clamp(tc, tc_min, tc_max ) - tc != vec2(0, 0)))
	{ FragmentColour = vec4(0, 0, 0, 0); }
	else
	{ FragmentColour = texture(tex, tc.xy); }
}

