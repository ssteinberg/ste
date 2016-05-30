
#type frag
#version 450

#define FXAA_PRESET 3
#define FXAA_GLSL_130
#include "nvidia_fxaa.glsl"

in v_out {
	vec2 uv;
	vec2 rcp_frame;
} vin;

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D tex;

void main() {
	gl_FragColor = vec4(FxaaPixelShader(vin.uv, tex, vin.rcp_frame), 1);
}
