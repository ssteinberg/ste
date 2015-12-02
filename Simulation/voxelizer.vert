
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

struct mesh_descriptor {
	mat4 model, transpose_inverse_model;
	int matIdx;
};

layout(std430, binding = 1) buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

out vs_out {
	vec3 N;
	vec2 st;
	flat int matIdx;
} vout;

uniform vec3 translation;
uniform mat4 trans_inverse_view_matrix;

void main() {
	mesh_descriptor md = mesh_descriptor_buffer[gl_DrawIDARB];

	mat4 model = md.model;
	mat4 trans_inverse_model = trans_inverse_view_matrix * md.transpose_inverse_model;
	
	vec4 P = model * vec4(vert, 1);
	P.xyz += translation;

	vout.N = (trans_inverse_model * vec4(normal, 1)).xyz;
	vout.st = tex_coords;
	vout.matIdx = md.matIdx;

    gl_Position = P;
}

