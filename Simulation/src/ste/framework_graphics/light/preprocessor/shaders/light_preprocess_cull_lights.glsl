
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_matrix_buffer.glsl"
#include "common.glsl"
#include "light.glsl"
#include "hdr_common.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 3) restrict writeonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(binding = 4) uniform atomic_uint ll_counter;
layout(shared, binding = 5) restrict writeonly buffer ll_data {
	uint16_t ll[];
};

layout(std430, binding = 6) restrict readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters hdr_params;
};

uniform vec4 np, rp, lp, tp, bp;

bool is_sphere_in_frustum(vec3 c, float r) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(rp.xyz, c) + rp.w > -r &&
			dot(lp.xyz, c) + lp.w > -r &&
			dot(tp.xyz, c) + tp.w > -r &&
			dot(bp.xyz, c) + bp.w > -r;
}

void main() {
	int light_idx = int(gl_GlobalInvocationID.x);
	if (light_idx >= light_buffer.length())
		return;

	light_descriptor ld = light_buffer[light_idx];

	// Transform light position/direction
	vec4 transformed_light_pos = vec4(light_transform(view_matrix_buffer.view_matrix, mat3(view_matrix_buffer.view_matrix), ld), 0);
	light_transform_buffer[light_idx] = transformed_light_pos;

	// Calculate cutoff based on HDR exposure
	float hdr_min_lum = intBitsToFloat(hdr_params.lum_min);
	float minimal_light_luminance = ld.luminance * .0000075f;
	float err = max(minimal_light_luminance, hdr_lum_to_luminance(hdr_min_lum));
	float range = light_calculate_effective_range(ld, err);

	// Frustum cull based on light effective range
	float r = range;
	vec3 c = transformed_light_pos.xyz;
	if (is_sphere_in_frustum(c, r)) {
		// Add light to active light linked list
		uint32_t ll_idx = atomicCounterIncrement(ll_counter);
		ll[ll_idx] = uint16_t(light_idx);

		// Zero out shadow face mask. It shall be computed later.
		light_buffer[light_idx].shadow_face_mask = 0;

		light_buffer[light_idx].effective_range = range;
		light_buffer[light_idx].minimal_luminance = minimal_light_luminance;
	}
}
