
#include <common.glsl>
#include <light.glsl>

#include <renderer_transform_buffers.glsl>

const int directional_light_cascades = 6;
const float cascade_projection_eye_distance = 5000.f;
const float cascade_projection_near_clip = 1000.f;

struct light_cascade_data {
	mat3x4 M;
	vec2 recp_vp;
	float proj_eye_dist;
	float proj_far_clip;
};

struct light_cascades_descriptor {
	light_cascade_data cascades[directional_light_cascades];
};

uniform cascades_depths_uniform_t {
	float cascades_depths[directional_light_cascades];
};

layout(std430, set=2, binding=12) restrict readonly buffer light_cascades_binding {
	light_cascades_descriptor light_cascades[max_active_directional_lights_per_frame];
};

/*
 *	Calculates the correct cascade number based on position in view-space.
 */
int light_which_cascade_for_position(vec3 position) {
	float z = -position.z;
	int cascade;
	for (cascade = 0; cascade < directional_light_cascades - 1; ++cascade) {
		if (z <= cascades_depths[cascade])
			break;
	}
	return cascade;
}

/*
 *	Reads the cascade descriptor index assigned to the light. Light must be a directional light, no checking
 *	is performed.
 */
uint light_get_cascade_descriptor_idx(light_descriptor ld) {
	return ld.polygonal_light_points_and_offset_or_cascade_idx;
}

/*
 *	Returns the shadowmap index for the specific light and cascade. Light must be a directional light.
 */
int light_get_cascade_shadowmap_idx(light_descriptor ld, int cascade) {
	return int(light_get_cascade_descriptor_idx(ld)) * directional_light_cascades + cascade;
}

/*
 *	Returns the cascade's near-clip plane distance of the viewing frustum.
 */
float light_cascade_near(int cascade) {
	return cascade == 0 ? projection_near_clip() : cascades_depths[cascade-1];
}

/*
 *	Returns the cascade's far-clip plane distance of the viewing frustum.
 */
float light_cascade_far(int cascade) {
	return cascades_depths[cascade];
}
