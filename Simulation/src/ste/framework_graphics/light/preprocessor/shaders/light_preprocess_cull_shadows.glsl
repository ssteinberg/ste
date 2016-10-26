
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"
#include "light.glsl"
#include "quaternion.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

uniform vec4 np, rp, lp, tp, bp;

const vec3 face_directions[6] = { vec3( 1, 0, 0),
								  vec3(-1, 0, 0),
								  vec3( 0, 1, 0),
								  vec3( 0,-1, 0),
								  vec3( 0, 0, 1),
								  vec3( 0, 0,-1) };
uniform float near = 20.f;

bool is_sphere_in_frustum(vec3 c, float r) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(rp.xyz, c) + rp.w > -r &&
			dot(lp.xyz, c) + lp.w > -r &&
			dot(tp.xyz, c) + tp.w > -r &&
			dot(bp.xyz, c) + bp.w > -r;
}

void main() {
	int ll_id = int(gl_GlobalInvocationID.x) / 6;
	if (ll_id >= ll_counter)
		return;

	int face = int(gl_GlobalInvocationID.x) % 6;
	uint16_t light_idx = ll[ll_id];
	light_descriptor ld = light_buffer[light_idx];
	
	// Nothing to do for directional lights
	if (ld.type == LightTypeDirectional)
		return;

	vec3 dir = quat_mul_vec(view_transform_buffer.view_transform.real, face_directions[face]);
	vec3 origin = ld.transformed_position;

	// Compute minimal bounding sphere of the shadow projection frustum
	float n = near;
	float f = max(ld.effective_range, n);
	float h = f - n;

	float f2 = f*f;
	float a = .5f * (h + (n*n - f2) / h);

	vec3 c = origin + dir * (f - a);
	float r = sqrt(f2 + a*a);

	if (is_sphere_in_frustum(c, r)) {
		int mask = 1 << face;
		atomicAdd(light_buffer[light_idx].shadow_face_mask, mask);
	}
}
