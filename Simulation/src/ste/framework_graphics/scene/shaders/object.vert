
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include "mesh_descriptor.glsl"
#include "girenderer_transform_buffer.glsl"
#include "quaternion.glsl"
#include "tangent_frame.glsl"

layout(location = 0) in vec4 tangent_frame_quat;
layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

out vec4 gl_Position;
out v {
	vec3 frag_position;
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int matIdx;
} vout;

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 wpos = model_transform(md, vert);
	vec3 spos = dquat_mul_vec(view_transform_buffer.view_transform, wpos);

	vec4 tangent_frame_transform = quat_mul_quat(view_transform_buffer.view_transform.real, md.tangent_transform_quat);
	mat3 tbn = extract_tangent_frame(tangent_frame_transform, tangent_frame_quat);

	vout.frag_position = spos.xyz;
	vout.frag_texcoords = tex_coords;
	vout.frag_tangent = tbn[0];
	vout.frag_normal = tbn[2];
	vout.matIdx = md.matIdx;

	gl_Position = project(spos);
}
