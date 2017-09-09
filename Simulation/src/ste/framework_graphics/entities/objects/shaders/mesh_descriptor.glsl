
struct mesh_descriptor {
	mat3x4 model_transform_matrix;
	vec4 tangent_transform_quat;

	vec4 bounding_sphere;

	int matIdx;
	int light_caster;
	
	float _unused0;
	float _unused1;
};

struct mesh_draw_params {
	uint count;
	uint first_index;
	int vertex_offset;

	float _unused;
};

vec3 transform_model(mesh_descriptor mesh, vec3 v) {
	return vec4(v, 1) * mesh.model_transform_matrix;
}

layout(std430, set=2, binding=2) restrict readonly buffer mesh_descriptors_binding {
	mesh_descriptor mesh_descriptor_buffer[];
};

layout(std430, set=2, binding=3) restrict readonly buffer mesh_draw_params_binding {
	mesh_draw_params mesh_draw_params_buffer[];
};
