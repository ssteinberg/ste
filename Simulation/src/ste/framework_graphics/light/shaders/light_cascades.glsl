
#include "common.glsl"
#include "light.glsl"

#include "girenderer_transform_buffer.glsl"

const int max_active_directional_lights_per_frame = 2;
const int directional_light_cascades = 6;

const float cascade_proj_near_clip = 25.f;
const float cascade_viewport_reserve = 1.075f;

const float light_cascade_minimal_distance = 1e+4;

struct light_cascade_descriptor {
	// Tangent and up vector of all cascades, with light direction being view vector
	vec4 X, Y;

	// cascade_data:
	// xy = reciprocal of viewport size
	// zw = cascade z limits in cascade-space
	vec4 cascades_data[directional_light_cascades];
};

/*
 *	Reads the cascade descriptor index assigned to the light. Light must be a directional light, no checking
 *	is performed.
 */
uint32_t light_get_cascade_descriptor_idx(light_descriptor ld) {
	return ld.cascade_idx;
}

/*
 *	Calculates the correct cascade number based on position in view-space.
 */
int light_which_cascade_for_position(vec3 position, float cascades_depths[directional_light_cascades]) {
	float z = -position.z;
	int cascade;
	for (cascade = 0; cascade < directional_light_cascades - 1; ++cascade) {
		if (z <= cascades_depths[cascade])
			break;
	}
	return cascade;
}

/*
 *	Returns the shadowmap index for the specific light and cascade. Light must be a directional light, 
 *	no checking is performed.
 */
int light_get_cascade_shadowmap_idx(light_descriptor ld, int cascade) {
	return int(ld.cascade_idx) * directional_light_cascades + cascade;
}

/*
 *	Returns the cascade's near-clip plane distance of the viewing frustum.
 */
float light_cascade_near(int cascade, float cascades_depths[directional_light_cascades]) {
	return cascade == 0 ? projection_near_clip() : cascades_depths[cascade-1];
}

/*
 *	Returns the cascade's far-clip plane distance of the viewing frustum.
 */
float light_cascade_far(int cascade, float cascades_depths[directional_light_cascades]) {
	return cascades_depths[cascade];
}

/*
 *	Calculates the cascade eye distance from the center of the cascade's viewing frustum segment.
 *	Chooses a cascade distance that gives enough range from the near-clip to the view frustum encompassed by the cascade
 *	to capture medium distance occluders, while maintaining depth precision.
 *
 *	Input params are the cascade z_limits
 */
float light_cascade_calculate_eye_dist(float z_far, float z_near) {
	float cascade_depth = z_near - z_far;
	return z_near + max3(2.f * cascade_proj_near_clip, 50.f * cascade_depth, light_cascade_minimal_distance);
}

/*
 *	Reads cascade's data:
 *	cascade_proj_far:	Far distance in cascade-space of the encompassed viewing frustum segment
 *	cascade_eye_dist:	Distance from cascade center (viewing frustum segment center) to the cascade eye
 *	recp_viewport:		Reciprocal of the cascade viewport
 */
void light_cascade_data(light_cascade_descriptor cascade_descriptor, 
						int cascade, 
						out float cascade_proj_far,
						out float cascade_eye_dist,
						out vec2 recp_viewport) {
	vec4 data = cascade_descriptor.cascades_data[cascade];

	vec2 z_limits = data.zw;
	float z_near = z_limits.x;
	float z_far = -z_limits.y;
	
	cascade_eye_dist = light_cascade_calculate_eye_dist(z_far, z_near);
	// The far end of the cascade, e.g. to cull out geometry when building cascade shadow map
	cascade_proj_far = -(cascade_eye_dist - z_far);
	
	recp_viewport = data.xy;
}

/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascade_eye_dist,
								vec2 recp_viewport,
								float near,
								float far) {
	vec3 center = vec3(0, 0, -mix(near,far,.5f));
	vec3 eye = center - l * cascade_eye_dist;
	
	vec3 X = cascade_descriptor.X.xyz;
	vec3 Y = cascade_descriptor.Y.xyz;

	mat3x4 M;
	M[0] = vec4(X, -dot(eye, X));
	M[1] = vec4(Y, -dot(eye, Y));
	M[2] = vec4(-l, dot(eye, l));

	M[0] *= recp_viewport.x;
	M[1] *= recp_viewport.y;

	return M;
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascade_eye_dist,
								vec2 recp_viewport,
								float cascades_depths[directional_light_cascades]) {
	float near = light_cascade_near(cascade, cascades_depths);
	float far = light_cascade_far(cascade, cascades_depths);
	
	return light_cascade_projection(cascade_descriptor, 
									cascade, 
									l, 
									cascade_eye_dist, 
									recp_viewport, 
									near, 
									far);
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascades_depths[directional_light_cascades],
								out float cascade_proj_far) {
	float cascade_eye_dist;
	vec2 recp_viewport;
	light_cascade_data(cascade_descriptor, 
					   cascade, 
					   cascade_proj_far, 
					   cascade_eye_dist, 
					   recp_viewport);

	return light_cascade_projection(cascade_descriptor, cascade, l, cascade_eye_dist, recp_viewport, cascades_depths);
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascades_depths[directional_light_cascades]) {
	float cascade_proj_far;
	return light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, cascade_proj_far);
}
