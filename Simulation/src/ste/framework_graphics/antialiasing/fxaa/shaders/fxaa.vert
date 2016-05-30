
#type vert
#version 450

layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

#include "girenderer_transform_buffer.glsl"

out v_out {
	vec2 uv;
	vec2 rcp_frame;
} vout;

out vec4 gl_Position;

const float FXAA_SUBPIX_SHIFT = 1.0f / 4.0f;

void main() {
	gl_Position = vec4(vert, 1);

	vout.rcp_frame = 1.f / vec2(backbuffer_size());
	vout.uv = tex_coords.xy;;
}
