
#type compute
#version 450

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"

#include "common.glsl"
#include "hdr_common.glsl"
#include "intersection.glsl"

#include "light.glsl"
#include "light_cascades.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(binding = 4) uniform atomic_uint ll_counter;
layout(shared, binding = 5) restrict writeonly buffer ll_data {
	uint ll[];
};
layout(shared, binding = 6) restrict writeonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

uniform vec4 np, rp, lp, tp, bp;

void add_to_lll(int light_idx) {
	uint ll_idx = atomicCounterIncrement(ll_counter);
	ll[ll_idx] = uint(light_idx);
}

void main() {
	int light_idx = int(gl_GlobalInvocationID.x);
	if (light_idx >= light_buffer.length())
		return;

	light_descriptor ld = light_buffer[light_idx];

	// Transform light position/direction
	vec3 transformed_light_pos = light_transform(view_transform_buffer.view_transform, ld);
	float range;

	bool add_light = false;

	if (ld.type == LightTypeDirectional) {
		// For directional lights:
		// Add light to active light linked list		
		uint cascade_idx = light_get_cascade_descriptor_idx(ld);
		
		// Compute orthonormal basis for light cascade space
		vec3 l = transformed_light_pos;
		vec3 x = cross(l, vec3(0,1,0));
		if (dot(x,x) < 1e-10)
			x = cross(l, vec3(1,0,0));
		x = normalize(x);
		vec3 y = normalize(cross(l,x));
		x = -x;		// Keep right-handed system
		
		// Write the basis to the cascade
		directional_lights_cascades[cascade_idx].X.xyz = x;
		directional_lights_cascades[cascade_idx].Y.xyz = y;
		
		add_light = true;
		range = inf;
	}
	else {
		// For spherical lights:
		// Calculate acceptable cutoff and range based on cutoff. Using those, check if cutoff sphere is in frustum,
		// in which case add to lll.
		float minimal_light_luminance = light_calculate_minimal_luminance(ld);
		range = light_calculate_effective_range(ld, minimal_light_luminance);

		// Frustum cull based on light effective range
		float r = range;
		vec3 c = transformed_light_pos.xyz;

		if (collision_sphere_infinite_frustum(c, r,
											  np, rp, lp, tp, bp)) {
			add_light = true;

			// Zero out shadow face mask. It shall be computed later.
			light_buffer[light_idx].shadow_face_mask = 0;
		}
	}

	if (add_light) {
		// Add light to active light linked list
		add_to_lll(light_idx);

		light_buffer[light_idx].transformed_position = transformed_light_pos;
		light_buffer[light_idx].effective_range = range;
	}
}
