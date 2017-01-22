
#include "common.glsl"
#include "light.glsl"

#include "girenderer_transform_buffer.glsl"

const int directional_light_cascades = 6;
const float cascade_viewport_reserve = 1.075f;

// Select a cascade eye distance that gives enough range from the near-clip to the view frustum encompassed by the cascade
// to capture medium distance occluders, while maintaining depth precision. Keep it constant amongst cascades to avoid shadow filtering
// artifacts when moving between cascades.
// The near clip distance also reflects this motivation.
const float cascade_projection_eye_distance = 10000.f;
const float cascade_projection_near_clip = 50.f;

const float cascades_thickness_multiplier_for_eye_dist = 5.f;

struct light_cascade_descriptor {
	// Tangent and up vector of all cascades, with light direction being view vector
	vec4 X, Y;

	// cascade_data:
	// xy = reciprocal of viewport size
	// z = projection eye distance
	// w = projection far clip
	vec4 cascades_data[directional_light_cascades];
};

/*
 *	Reads the cascade descriptor index assigned to the light. Light must be a directional light, no checking
 *	is performed.
 */
uint light_get_cascade_descriptor_idx(light_descriptor ld) {
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
	
	recp_viewport = data.xy;
	cascade_eye_dist = data.z;
	// The far and near clips of the cascades
	cascade_proj_far = data.w;
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
	
	vec3 X = cascade_descriptor.X.xyz * recp_viewport.x;
	vec3 Y = cascade_descriptor.Y.xyz * recp_viewport.y;

	mat3x4 M = mat3x4(
		vec4(X, -dot(eye, X)),
		vec4(Y, -dot(eye, Y)),
		vec4(-l, dot(eye, l))
	);

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
								out vec2 cascade_recp_viewport,
								out float cascade_proj_far) {
	float cascade_eye_dist;
	light_cascade_data(cascade_descriptor, 
					   cascade, 
					   cascade_proj_far, 
					   cascade_eye_dist, 
					   cascade_recp_viewport);

	return light_cascade_projection(cascade_descriptor, cascade, l, cascade_eye_dist, cascade_recp_viewport, cascades_depths);
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascades_depths[directional_light_cascades],
								out float cascade_proj_far) {
	vec2 cascade_recp_viewport;
	return light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, cascade_recp_viewport, cascade_proj_far);
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascades_depths[directional_light_cascades],
								out vec2 cascade_recp_viewport) {
	float cascade_proj_far;
	return light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, cascade_recp_viewport, cascade_proj_far);
}
/*
 *	Constructs a cascade's transformation and projection matrix.
 */
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascades_depths[directional_light_cascades]) {
	float cascade_proj_far;
	vec2 cascade_recp_viewport;
	return light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, cascade_recp_viewport, cascade_proj_far);
}
