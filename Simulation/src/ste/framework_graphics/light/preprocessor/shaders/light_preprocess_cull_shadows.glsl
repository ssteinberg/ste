
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"

#include "intersection.glsl"
#include "quaternion.glsl"

#include "light.glsl"

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
uniform float shadow_proj_near = 20.f;

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
	float n = shadow_proj_near;
	float f = max(ld.effective_range, n);
	float h = f - n;

	float f2 = f*f;
	float a = .5f * (h + (n*n - f2) / h);

	vec3 c = origin + dir * (f - a);
	float r = sqrt(f2 + a*a);

	if (collision_sphere_infinite_frustum(c, r,
										  np, rp, lp, tp, bp)) {
		int mask = 1 << face;
		atomicAdd(light_buffer[light_idx].shadow_face_mask, mask);
	}
}
