
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

struct mesh_descriptor {
	mat4 model, transpose_inverse_model;
};

layout(std430, binding = 1) buffer mesh_data {
	mesh_descriptor mesh_transform_matrices[];
};

out vec4 gl_Position;
out vec3 frag_position;
out float frag_depth;
out vec2 frag_texcoords;
out vec3 frag_normal;
out vec3 frag_tangent;
flat out int drawId;

uniform mat4 projection;
uniform mat4 view_matrix;
uniform mat4 trans_inverse_view_matrix;
uniform float near = 5.0f;
uniform float far = 10000.0f;

void main() {
	mat4 view_model = view_matrix * mesh_transform_matrices[gl_DrawIDARB].model;
	mat4 trans_inverse_view_model = trans_inverse_view_matrix * mesh_transform_matrices[gl_DrawIDARB].transpose_inverse_model;

	vec4 eye_v = view_model * vec4(vert, 1);

	frag_position = eye_v.xyz;
	frag_texcoords = tex_coords;
	frag_normal = (trans_inverse_view_model * vec4(normal, 1)).xyz;
	frag_tangent = (view_model * vec4(tangent, 1)).xyz;
	drawId = gl_DrawIDARB;

	gl_Position = projection * eye_v;
	
	frag_depth = (-eye_v.z - near) / (far - near);
}
