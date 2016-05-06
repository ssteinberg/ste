
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "light.glsl"
#include "linked_light_lists.glsl"
#include "linked_light_lists_store.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};
layout(shared, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};
layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(r32ui, binding = 6) restrict uniform uimage2D lll_heads;
layout(shared, binding = 7) restrict buffer lll_counter_data {
	uint32_t lll_counter;
};
layout(shared, binding = 11) restrict writeonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(binding = 11) uniform sampler2DShadow depth_map;

uniform float near, aspect;
uniform float two_near_tan_fovy_over_two;	// 2 * near * tan(fovy * .5)
uniform float proj22, proj23;

const int res_multiplier = 8;

void main() {
	vec2 bb_size = textureSize(depth_map, 0).xy / float(res_multiplier);

	ivec2 image_coord = ivec2(gl_FragCoord.xy);
	vec2 frag_coords = vec2(gl_FragCoord.xy) / bb_size;

	vec2 ndc = frag_coords - vec2(.5f);
	float near_plane_h = two_near_tan_fovy_over_two;
	float near_plane_w = near_plane_h * aspect;
	vec2 near_plane_pos = ndc * vec2(near_plane_w, near_plane_h);

	vec3 l = vec3(near_plane_pos, -near);
	float a = dot(l, l);

	lll_element active_lights[max_active_lights_per_frame];
	uint32_t total_active_lights = 0;

	for (int ll_i = 0; ll_i < ll_counter && total_active_lights < max_active_lights_per_frame; ++ll_i) {
		uint16_t light_idx = ll[ll_i];
		light_descriptor ld = light_buffer[light_idx];

		vec3 c = light_transform_buffer[light_idx].xyz;
		float r = ld.effective_range * 1.05f;

		vec3 _c = -c;
		float b = dot(l, _c);

		float delta = b*b - a * (dot(c, c) - r*r);
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

					float d00 = texture(depth_map, vec3(frag_coords + vec2(.0f) 					/ bb_size, zf)).x;
					float d10 = texture(depth_map, vec3(frag_coords + vec2(res_multiplier - 1, .0f) / bb_size, zf)).x;
					float d01 = texture(depth_map, vec3(frag_coords + vec2(.0f, res_multiplier - 1) / bb_size, zf)).x;
					float d11 = texture(depth_map, vec3(frag_coords + vec2(res_multiplier - 1) 		/ bb_size, zf)).x;
					if (d00 + d10 + d01 + d11 > .5f)
						add_point = true;
				}
			}

			if (add_point) {
				active_lights[total_active_lights] = lll_encode(light_idx,
																ll_i,
																z_min,
																z_max);

				++total_active_lights;
			}
		}
	}

	uint32_t next_idx = atomicAdd(lll_counter, total_active_lights + 1);
	imageStore(lll_heads, image_coord, next_idx.xxxx);

	for (uint32_t i = 0; i < total_active_lights; ++i)
		lll_buffer[next_idx + i] = active_lights[i];
	lll_buffer[next_idx + total_active_lights].data.x = uintBitsToFloat(0xFFFFFFFF);
}
