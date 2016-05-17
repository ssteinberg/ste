
#type vert
#version 450

#include "girenderer_transform_buffer.glsl"

layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

out vec4 gl_Position;
out vec3 frag_position;
out vec2 frag_texcoords;

uniform mat4 projection;
const float far = 100000.0f;

void main() {
	vec3 p = vert * far * .99f + vec3(0, -far * .05f, 0);
	vec4 eye_v = vec4(dquat_mul_vec(view_transform_buffer.view_transform, p), 1);

	frag_position = eye_v.xyz;
	frag_texcoords = tex_coords;

	gl_Position = projection * eye_v;
}
