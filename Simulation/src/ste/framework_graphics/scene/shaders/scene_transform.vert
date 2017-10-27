
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include <mesh_descriptor.glsl>
#include <renderer_transform_buffers.glsl>
#include <quaternion.glsl>
#include <tangent_frame.glsl>

layout(location = 0) in vec4 tangent_frame_quat;
layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

layout(location = 0) out scene_transform {
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int material_id;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 wpos = transform_model(md, vert);
	vec3 spos = transform_view(wpos);

	vec4 tangent_frame_transform = quat_mul_quat(view_transform_buffer.view_transform.real, md.tangent_transform_quat);
	mat3 tbn = extract_tangent_frame(tangent_frame_transform, tangent_frame_quat);

	vout.frag_texcoords = tex_coords;
	vout.frag_tangent = tbn[0];
	vout.frag_normal = tbn[2];
	vout.material_id = md.material_id;

	gl_Position = project(spos);
}
