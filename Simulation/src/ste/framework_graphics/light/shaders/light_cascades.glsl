
#include "common.glsl"
#include "light.glsl"

#include "girenderer_transform_buffer.glsl"

const int max_active_directional_lights_per_frame = 4;
const int directional_light_cascades = 6;

struct light_cascade_descriptor {
	// Tangent and up vector of all cascades, with light direction being view vector
	vec4 X, Y;

	// cascade_data:
	// xy = (near, far) in cascade space
	// z = cascade eye distance
	// w = reciprocal of viewport size
	vec4 cascades_data[directional_light_cascades];

	// Cascades partitioning of the view frustum
	float cascades_far[directional_light_cascades];
};

uint32_t light_get_cascade_descriptor_idx(light_descriptor ld) {
	return ld.cascade_idx;
}

int light_which_cascade_for_position(light_cascade_descriptor cascade_descriptor, vec3 position) {
	float z = -position.z;
	int cascade;
	for (cascade = 0; cascade < directional_light_cascades - 1; ++cascade) {
		if (z <= cascade_descriptor.cascades_far[cascade])
			break;
	}
	return cascade;
}

int light_get_cascade_shadowmap_idx(light_descriptor ld, int cascade) {
	return int(ld.cascade_idx) * directional_light_cascades + cascade;
}

float light_cascade_near(light_cascade_descriptor cascade_descriptor, int cascade) {
	return cascade == 0 ? projection_near_clip() : cascade_descriptor.cascades_far[cascade-1];
}

float light_cascade_far(light_cascade_descriptor cascade_descriptor, int cascade) {
	return cascade_descriptor.cascades_far[cascade];
}

vec2 light_cascade_zlimits(light_cascade_descriptor cascade_descriptor, int cascade) {
	vec4 data = cascade_descriptor.cascades_data[cascade];
	return data.xy;
}

void light_cascade_data(light_cascade_descriptor cascade_descriptor, 
						int cascade, 
						out vec2 z_limits,
						out float cascade_eye_dist,
						out float recp_viewport) {
	vec4 data = cascade_descriptor.cascades_data[cascade];
	z_limits = data.xy;
	cascade_eye_dist = data.z;
	recp_viewport = data.w;
}

mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascade_eye_dist,
								float recp_viewport,
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

	M[0] *= recp_viewport;
	M[1] *= recp_viewport;

	return M;
}
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								float cascade_eye_dist,
								float recp_viewport) {
	float near = light_cascade_near(cascade_descriptor, cascade);
	float far = light_cascade_far(cascade_descriptor, cascade);
	
	return light_cascade_projection(cascade_descriptor, cascade, l, cascade_eye_dist, recp_viewport, near, far);
}
mat3x4 light_cascade_projection(light_cascade_descriptor cascade_descriptor, 
								int cascade,
								vec3 l,
								out vec2 z_limits) {
	float cascade_eye_dist;
	float recp_viewport;
	light_cascade_data(cascade_descriptor, cascade, z_limits, cascade_eye_dist, recp_viewport);

	return light_cascade_projection(cascade_descriptor, cascade, l, cascade_eye_dist, recp_viewport);
}
