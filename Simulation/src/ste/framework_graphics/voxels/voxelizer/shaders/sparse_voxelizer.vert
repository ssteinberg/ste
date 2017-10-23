
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include <mesh_descriptor.glsl>
#include <renderer_transform_buffers.glsl>
#include <quaternion.glsl>
#include <tangent_frame.glsl>

layout(location = 0) in vec4 _unused;
layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

layout(location = 0) out scene_transform {
	vec2 frag_texcoords;
	flat int material_id;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 wpos = transform_model(md, vert);

	vout.frag_texcoords = tex_coords;
	vout.material_id = md.material_id;

	gl_Position = vec4(wpos, 0);
}
