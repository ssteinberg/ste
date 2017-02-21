
#include <light.glsl>

struct light_shading_parameters {
	light_descriptor ld;
	uint light_id;
	uint ll_id;

	// Light distance and direction
	float l_dist;
	vec3 l;

	// Light illuminance arriving at shaded sample
	vec3 cd_m2;
};

struct fragment_shading_parameters {
	// Screen space coords
	ivec2 coords;

	// View space position, normal, tangent, bi-tangent
	vec3 p;
	vec3 n;
	vec3 t;
	vec3 b;

	// Normalized ray from fragment to eye, view space
	vec3 v;

	// World space position, normal
	vec3 world_position;
	vec3 world_normal;
	vec3 world_v;
};
