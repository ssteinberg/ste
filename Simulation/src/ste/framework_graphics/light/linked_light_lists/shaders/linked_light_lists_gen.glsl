
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#include "light.glsl"
#include "linked_light_lists.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};
layout(std430, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};
layout(std430, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(std430, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(r32ui, binding = 6) restrict uniform uimage2D lll_heads;
layout(binding = 5) uniform atomic_uint lll_counter;
layout(std430, binding = 11) restrict writeonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(binding = 11) uniform sampler2D depth_map;

uniform float near, aspect;
uniform float two_tan_fovy_over_two;	// 2 * tan(fovy * .5)
uniform float proj22, proj23;

void main() {
	int id = int(gl_GlobalInvocationID.z);
	ivec2 frag_coord = ivec2(gl_GlobalInvocationID.xy);

	ivec2 lllsize = imageSize(lll_heads);
	if (id >= ll_counter ||
		frag_coord.x >= lllsize.x ||
		frag_coord.y >= lllsize.y)
		return;

	uint16_t light_idx = ll[id];
	light_descriptor ld = light_buffer[light_idx];

	vec3 c = light_transform_buffer[light_idx].xyz;
	float r = ld.effective_range;

	vec2 fcoords = vec2(frag_coord) / vec2(lllsize);
	vec2 ndc = fcoords - vec2(.5f);

	float near_plane_h = near * two_tan_fovy_over_two;
	float near_plane_w = near_plane_h * aspect;
	vec2 near_plane_pos = ndc * vec2(near_plane_w, near_plane_h);

	vec3 o = vec3(.0f);
	vec3 l = vec3(near_plane_pos, -near);

	vec3 o_c = o - c;
	float a = dot(l, l);
	float b = dot(l, o_c);

	float delta = b*b - a * (dot(o_c, o_c) - r*r);
	if (delta > 0) {
		float sqrt_delta = sqrt(delta);
		float z_max = l.z * (-b - sqrt_delta) / a;
		float z_min = l.z * (-b + sqrt_delta) / a;

		bool add_point = false;

		if (z_min < -near) {
			if (z_max >= -near) {
				// Origin is inside the light radius
				add_point = true;
			}
			else {
				// Compare against depth buffer
				float ndc_zf = proj22 + proj23 / z_max;
				float zf = (ndc_zf + 1.f) * .5f;

				float d = texelFetch(depth_map, frag_coord * 4, 0).x;
				if (d > zf)
					add_point = true;
			}
		}

		if (add_point) {
			uint32_t next_idx = atomicCounterIncrement(lll_counter);
			uint32_t prev_head = imageLoad(lll_heads, frag_coord).x;
			imageStore(lll_heads, frag_coord, next_idx.xxxx);

			lll_element l;
			l.light_idx = uint16_t(light_idx);
			l.ll_idx = uint16_t(id);
			l.next_ptr = prev_head;
			l.z_min = z_min;
			l.z_max = z_max;

			lll_buffer[next_idx] = l;
		}
	}
}
