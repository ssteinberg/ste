
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include "mesh_descriptor.glsl"

layout(location = 0) in vec3 vert;

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

out vs_out {
	flat int instanceIdx;
	flat uint drawIdx;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	mat4 view_model = md.model;

	gl_Position = view_model * vec4(vert, 1);
	vout.instanceIdx = gl_InstanceID;
	vout.drawIdx = draw_id;
}
