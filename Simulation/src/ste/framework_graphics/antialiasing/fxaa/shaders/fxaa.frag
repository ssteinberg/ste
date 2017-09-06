
#type frag
#version 450

#define FXAA_PRESET 3

#include <nvidia_fxaa.glsl>

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D input_tex;

layout(location = 0) out vec4 frag_color;

void main() {
	vec2 rcp_frame = 1.f / textureSize(input_tex, 0);
	frag_color = vec4(FxaaPixelShader(uv, input_tex, rcp_frame), 1);
}
