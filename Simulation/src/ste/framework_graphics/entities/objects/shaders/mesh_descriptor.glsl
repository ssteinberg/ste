
struct mesh_descriptor {
	mat4 model;
	mat4 transpose_inverse_model;

	vec4 bounding_sphere;

	int mat_idx;

	uint count;
	uint first_index;
	uint base_vertex;
};
