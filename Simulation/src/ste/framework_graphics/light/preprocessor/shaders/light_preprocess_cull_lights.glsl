
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"

#include "common.glsl"
#include "hdr_common.glsl"
#include "intersection.glsl"

#include "light.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(binding = 4) uniform atomic_uint ll_counter;
layout(shared, binding = 5) restrict writeonly buffer ll_data {
	uint16_t ll[];
};

uniform vec4 np, rp, lp, tp, bp;

void add_to_lll(int light_idx) {
	uint32_t ll_idx = atomicCounterIncrement(ll_counter);
	ll[ll_idx] = uint16_t(light_idx);
}

void main() {
	int light_idx = int(gl_GlobalInvocationID.x);
	if (light_idx >= light_buffer.length())
		return;

	light_descriptor ld = light_buffer[light_idx];

	// Transform light position/direction
	vec3 transformed_light_pos = light_transform(view_transform_buffer.view_transform, ld);

	if (ld.type == LightTypeDirectional) {
		// For directional lights: Always add to lll
		
		// Add light to active light linked list
		add_to_lll(light_idx);
		
		light_buffer[light_idx].transformed_position = transformed_light_pos;
	}
	else {
		// For spherical lights: Calculate acceptable cutoff and range based on cutoff. 
		// Using those, check if cutoff sphere is in frustum.
		float minimal_light_luminance = ld.luminance * .000005f;
		float range = light_calculate_effective_range(ld, minimal_light_luminance);

		// Frustum cull based on light effective range
		float r = range;
		vec3 c = transformed_light_pos.xyz;

		if (collision_sphere_infinite_frustum(c, r,
											  np, rp, lp, tp, bp)) {
			// Add light to active light linked list
			add_to_lll(light_idx);

			// Zero out shadow face mask. It shall be computed later.
			light_buffer[light_idx].shadow_face_mask = 0;

			light_buffer[light_idx].effective_range = range;
			light_buffer[light_idx].transformed_position = transformed_light_pos;
			light_buffer[light_idx].minimal_luminance = minimal_light_luminance;
		}
	}
}
