
struct mesh_descriptor {
	mat4 model;
	mat4 transpose_inverse_model;

	vec4 bounding_sphere;

	int matIdx;

	float _unused[3];
};

struct mesh_draw_params {
	uint count;
	uint first_index;
	uint base_vertex;

	float _unused;
};
