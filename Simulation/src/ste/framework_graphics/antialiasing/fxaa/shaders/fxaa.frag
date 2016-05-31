
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#define FXAA_PRESET 3
#define FXAA_GLSL_130
#include "nvidia_fxaa.glsl"

in v_out {
	vec2 uv;
	vec2 rcp_frame;
} vin;

out vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D input;

void main() {
	gl_FragColor = vec4(FxaaPixelShader(vin.uv, input, vin.rcp_frame), 1);
}
