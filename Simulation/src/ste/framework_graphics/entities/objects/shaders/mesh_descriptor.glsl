
#include "dual_quaternion.glsl"

struct mesh_descriptor {
	dual_quaternion model_transform;

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
