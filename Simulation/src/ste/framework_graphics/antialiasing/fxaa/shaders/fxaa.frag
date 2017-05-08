
#type frag
#version 450

#define FXAA_PRESET 3
#define FXAA_GLSL_130
#include <nvidia_fxaa.glsl>

in v_out {
	vec2 uv;
	vec2 rcp_frame;
} vin;

layout(location = 0) uniform sampler2D input_tex;

out vec4 frag_color;

void main() {
	frag_color = vec4(FxaaPixelShader(vin.uv, input_tex, vin.rcp_frame), 1);
}
