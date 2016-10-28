
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"
#include "common.glsl"
#include "light.glsl"
#include "hdr_common.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(binding = 4) uniform atomic_uint ll_counter;
layout(shared, binding = 5) restrict writeonly buffer ll_data {
	uint16_t ll[];
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

	// Calculate cutoff
	float minimal_light_luminance = ld.luminance * .000005f;
	float range;

	bool add_light = false;

	// Transform light position/direction
	vec3 transformed_light_pos = light_transform(view_transform_buffer.view_transform, ld);

	if (ld.type == LightTypeDirectional) {
		// For directional lights: Infinite range, no cutoff and always add
		minimal_light_luminance = .0f;
		range = inf;

		add_light = true;
	}
	else {
		// For spherical lights: Calculate acceptable cutoff and range based on cutoff. 
		// Using those, check if cutoff sphere is in frustum.
		minimal_light_luminance = ld.luminance * .000005f;
		range = light_calculate_effective_range(ld, minimal_light_luminance);

		// Frustum cull based on light effective range
		float r = range;
		vec3 c = transformed_light_pos.xyz;

		add_light = is_sphere_in_frustum(c, r);
	}

	if (add_light) {
		// Add light to active light linked list
		uint32_t ll_idx = atomicCounterIncrement(ll_counter);
		ll[ll_idx] = uint16_t(light_idx);

		// Zero out shadow face mask. It shall be computed later.
		light_buffer[light_idx].shadow_face_mask = 0;

		light_buffer[light_idx].effective_range = range;
		light_buffer[light_idx].transformed_position = transformed_light_pos;
		light_buffer[light_idx].minimal_luminance = minimal_light_luminance;
	}
}
