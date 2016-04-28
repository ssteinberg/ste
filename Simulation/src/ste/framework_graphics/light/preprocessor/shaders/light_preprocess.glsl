
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "light.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(std430, binding = 3) restrict writeonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(binding = 4) uniform atomic_uint ll_counter;
layout(std430, binding = 5) restrict writeonly buffer ll_data {
	uint16_t ll[];
};

uniform mat4 view_matrix;
uniform vec4 np, fp, rp, lp, tp, bp;

bool is_sphere_in_frustum(vec3 c, float r) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(fp.xyz, c) + fp.w > -r &&
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
	vec4 transformed_light_pos = light_transform(view_matrix, mat3(view_matrix), ld);
	light_transform_buffer[light_idx] = transformed_light_pos;

	// Frustum cull based on light effective range
	float r = ld.effective_range;
	vec3 c = transformed_light_pos.xyz;
	if (is_sphere_in_frustum(c, r)) {
		// Add light to active light linked list
		uint32_t ll_idx = atomicCounterIncrement(ll_counter);
		ll[ll_idx] = uint16_t(light_idx);

		// Zero out shadow face mask. It shall be computed later.
		light_buffer[light_idx].shadow_face_mask = 0;
	}
}
