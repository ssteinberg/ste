
struct mesh_descriptor {
	mat3x4 model_transform_matrix;
	vec4 tangent_transform_quat;

	vec4 bounding_sphere;

	int matIdx;
	int light_caster;

	float _unused[2];
};

struct mesh_draw_params {
	uint count;
	uint first_index;
	uint base_vertex;

	float _unused;
};

vec3 model_transform(mesh_descriptor mesh, vec3 v) {
	return vec4(v, 1) * mesh.model_transform_matrix;
}
