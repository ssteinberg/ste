
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

#include "common.glsl"

#include "light.glsl"
#include "linked_light_lists.glsl"
#include "linked_light_lists_store.glsl"

#include "girenderer_transform_buffer.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint ll[];
};

layout(r8ui,  binding = 4) restrict writeonly uniform uimage2D lll_size;
layout(r8ui,  binding = 5) restrict writeonly uniform uimage2D lll_low_detail_size;
layout(r32ui, binding = 6) restrict writeonly uniform uimage2D lll_heads;
layout(r32ui, binding = 7) restrict writeonly uniform uimage2D lll_low_detail_heads;
layout(shared, binding = 7) restrict buffer lll_counter_data {
	uint lll_counter;
};
layout(shared, binding = 11) restrict writeonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(bindless_sampler) uniform sampler2D depth_map;

lll_element active_lights[total_max_active_lights_per_frame];
lll_element active_low_detail_lights[max_active_low_detail_lights_per_frame];
int total_active_lights = 0;
int total_active_low_detail_lights = 0;

vec2 compute_depth_range(float delta, float a, float b, vec3 l) {
	float near = projection_near_clip();

	float sqrt_delta = sqrt(delta);
	float z_max = min(l.z * (-b - sqrt_delta) / a, -near);
	float z_min = min(l.z * (-b + sqrt_delta) / a, -near);

	vec2 d;
	d.x = project_depth(z_min);		// depth of z_min
	d.y = project_depth(z_max);		// depth of z_max

	return d;
}

void add_active_light(uint light_idx, 
					  uint ll_i, 
					  vec2 depth_range) {
	active_lights[total_active_lights++] = lll_encode(light_idx, ll_i, depth_range);
}

void add_low_detail_light(uint light_idx, 
						  uint ll_i, 
						  vec2 depth_range) {
	if (total_active_low_detail_lights >= max_active_low_detail_lights_per_frame)
		return;

	active_low_detail_lights[total_active_low_detail_lights++] = lll_encode(light_idx, ll_i, depth_range);
}

void main() {
	ivec2 image_coord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 image_size = imageSize(lll_heads);
	if (any(greaterThanEqual(image_coord, image_size)))
		return;

	vec2 frag_coords = (vec2(gl_GlobalInvocationID.xy) + vec2(.5f)) * float(lll_image_res_multiplier) / vec2(backbuffer_size());
	int depth_lod = lll_depth_lod;

	float near = projection_near_clip();
	vec2 ndc = frag_coords * 2.f - vec2(1.f);

	float near_plane_h = projection_tan_half_fovy() * near;
	float near_plane_w = near_plane_h * projection_aspect();
	vec2 near_plane_pos = ndc * vec2(near_plane_w, near_plane_h);

	vec3 l = vec3(near_plane_pos, -near);
	float a = dot(l, l);

	for (int j = 0; j < ll_counter && total_active_lights < total_max_active_lights_per_frame; ++j) {
		uint ll_i = uint(j);
		uint light_idx = ll[ll_i];
		light_descriptor ld = light_buffer[light_idx];

		if (light_type_is_directional(ld.type)) {
			// For directional lights: Nothing to do, add point
			vec2 depth_range = vec2(.0f, 1.f);
			add_active_light(light_idx, ll_i, depth_range);
			add_low_detail_light(light_idx, ll_i, depth_range);
		}
		else {
			// For spherical lights: Check to see that fragment's stored depth intersects the lights cutoff sphere.
			vec3 c = ld.transformed_position;
			float r = light_effective_range(ld) * 1.01f;	// Slightly increase radius due to lll storage imprecision

			float b = dot(l, -c);
			float b2 = b*b;
			float c2 = dot(c, c);

			float delta = b2 - a * (c2 - r*r);
			if (delta > 0) {
				vec2 depth_range = compute_depth_range(delta, a, b, l);

				if (depth_range.x < 1.f) {
					// Compare against depth buffer
					float d = texelFetch(depth_map, image_coord, depth_lod).x;
					if (d <= depth_range.y) {
						add_active_light(light_idx, ll_i, depth_range);
						
						// If the fragment in contained in the light effective sphere, calculate also the range for the low detail sphere. 
						// The low detail range is used for different process light volumetric lighting.
						float low_r = lll_low_detail_light_effective_range(ld);
						float low_delta = b2 - a * (c2 - low_r*low_r);

						if (low_delta > 0) {
							vec2 low_depth_range = compute_depth_range(low_delta, a, b, l);
							add_low_detail_light(light_idx, ll_i, low_depth_range);
						}
					}
				}
			}
		}
	}

	// Add the encoded lights to the per-pixel linked-light-list
	uint next_active_idx = atomicAdd(lll_counter, uint(total_active_lights + total_active_low_detail_lights));
	uint next_low_detail_idx = next_active_idx + total_active_lights;
	imageStore(lll_heads, image_coord, next_active_idx.xxxx);
	imageStore(lll_low_detail_heads, image_coord, next_low_detail_idx.xxxx);
	
	// Copy lll elements to buffer and mark the ll end
	for (int i = 0; i < total_active_lights; ++i)
		lll_buffer[next_active_idx + i] = active_lights[i];

	for (int i = 0; i < total_active_low_detail_lights; ++i)
		lll_buffer[next_low_detail_idx + i] = active_low_detail_lights[i];
		
	// Write linked-lists' lengths
	imageStore(lll_size, image_coord, total_active_lights.xxxx);
	imageStore(lll_low_detail_size, image_coord, total_active_low_detail_lights.xxxx);
}
