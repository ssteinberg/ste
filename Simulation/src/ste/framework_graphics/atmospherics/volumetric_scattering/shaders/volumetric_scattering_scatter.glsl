
#type compute
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

#include "volumetric_scattering.glsl"
#include "atmospherics.glsl"

#include "light_transport.glsl"
#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "girenderer_transform_buffer.glsl"
#include "project.glsl"

#include "fast_rand.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(r8ui,  binding = 5) restrict readonly uniform uimage2D lll_size;
layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(shared, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(shared, binding = 7) restrict readonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

layout(rgba16f, binding = 7) restrict uniform image3D volume;

#include "linked_light_lists_load.glsl"

layout(bindless_sampler) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(bindless_sampler) uniform sampler2DArrayShadow directional_shadow_depth_maps;

layout(bindless_sampler) uniform sampler2DArray atmospheric_optical_length_lut;

layout(bindless_sampler) uniform sampler2D depth_map;

uniform float cascades_depths[directional_light_cascades];

const float samples = 2.f;

void depth_limits_3x3(vec2 uv, int lod, out float mind, out float maxd) {
	vec2 d00 = textureLodOffset(depth_map, uv, lod, ivec2(-1,-1)).xy;
	vec2 d10 = textureLodOffset(depth_map, uv, lod, ivec2( 0,-1)).xy;
	vec2 d20 = textureLodOffset(depth_map, uv, lod, ivec2( 1,-1)).xy;
	vec2 d01 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 0)).xy;
	vec2 d11 = textureLod(depth_map, uv, lod).xy;
	vec2 d21 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 0)).xy;
	vec2 d02 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 1)).xy;
	vec2 d12 = textureLodOffset(depth_map, uv, lod, ivec2( 0, 1)).xy;
	vec2 d22 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 1)).xy;
	
	float mind1 = min(min(d00.x, d10.x), min(d20.x, d01.x));
	float mind2 = min(min(d11.x, d21.x), min(d02.x, d12.x));
	mind = min3(mind1, mind2, d22.x);

	float maxd1 = min(min(d00.y, d10.y), min(d20.y, d01.y));
	float maxd2 = min(min(d11.y, d21.y), min(d02.y, d12.y));
	maxd = max3(maxd1, maxd2, d22.y);
}

vec2 seed_scattering(vec2 slice_coords, uint light_idx, float s, float depth) {
	return slice_coords + vec2(light_idx + s * .1f, depth);
}

vec2 slice_coords_to_fragcoords(vec2 v) {
	return (v + vec2(.5f)) * float(volumetric_scattering_tile_size) / vec2(backbuffer_size());
}

vec3 calculate_scattering_position(vec2 seed, vec2 slice_coords, float s, float z_start, float z_next) {
	// Generate a random number, and use it to generate a pseudo-random sampling point
	float r = fast_rand(seed);

	// Divide the depth into k slices, where k=samples.
	// Sample uniformly from each slice
	float sample_slice_depth = (z_next - z_start) / samples;
	float a = z_start + sample_slice_depth * s;
	float b = a + sample_slice_depth;

	float z = mix(a, b, r * .99999f);

	// Use some noise to generate 2 more pseudo-random numbers from r. Use them to generate the xy sampling coordinates.
	vec2 noise = vec2(65198.10937f, 22109.532971f);
	vec2 jitter = fract(r * noise);
	vec2 coords = slice_coords_to_fragcoords(vec2(slice_coords) + jitter);

	// Unproject generated coordinates into eye space
	return unproject_screen_position_with_z(z, coords);
}

vec3 scatter_spherical_light(vec2 slice_coords,
							 float depth,
							 float z_start,
							 float z_next,
							 float thickness,
							 uint light_idx,
							 light_descriptor ld,
							 uint shadowmap_idx,
							 float min_lum) {
	vec3 rgb = vec3(.0f);
	for (float s = 0; s < samples; ++s) {
		vec3 position = calculate_scattering_position(seed_scattering(slice_coords, light_idx, s, depth),
													  slice_coords,
													  s, z_start, z_next);
		vec3 w_pos = transform_view_to_world_space(position);
				
		vec3 l = light_incidant_ray(ld, position);
		float l_dist = length(l);
		l /= l_dist;
					
		vec3 shadow_v = w_pos- ld.position;
		/*float shadow = shadow_fast(shadow_depth_maps,
								   shadow_maps,
								   shadowmap_idx,
								   position,
								   l,
								   shadow_v,
								   ld.radius,
								   ivec2(slice_coords));*/
		float shadow = shadow_test(shadow_depth_maps,
								   shadowmap_idx,
								   shadow_v,
								   ld.radius);

		if (shadow <= .0f)
			continue;

		float scaling_size = thickness;
		float scale = min(l_dist, scaling_size) / scaling_size;

		rgb += irradiance(ld, l_dist) * scatter(ld.position, w_pos, eye_position(),
												thickness,
												atmospheric_optical_length_lut);
	}

	return rgb / float(samples);
}

vec3 scatter_directional_light(vec2 slice_coords,
							   float depth,
							   float z_start,
							   float z_next,
							   float thickness,
							   uint light_idx,
							   light_descriptor ld,
							   mat3x4 M,
							   int shadowmap_idx,
							   float min_lum) {
	vec3 rgb = vec3(.0f);
	for (float s = 0; s < samples; ++s) {
		vec3 position = calculate_scattering_position(seed_scattering(slice_coords, light_idx, s, depth),
													  slice_coords,
													  s, z_start, z_next);
		vec3 w_pos = transform_view_to_world_space(position);
				
		vec3 l = light_incidant_ray(ld, position);					
		float shadow = shadow_test(directional_shadow_depth_maps,
								   shadowmap_idx,
								   position,
								   M);
		if (shadow <= .0f)
			continue;
			
		rgb += irradiance(ld, .0f) * scatter_ray(eye_position(), w_pos, -ld.position,
												 thickness,
												 atmospheric_optical_length_lut);
	}

	return rgb / float(samples);
}

vec3 scatter(float depth, float depth_next_tile, 
			 ivec2 slice_coords, vec2 fragcoords, vec2 next_tile_fragcoords,
			 light_descriptor ld, uint light_idx, uint ll_idx, float min_lum,
			 light_cascade_descriptor cascade_descriptor, inout float current_cascade_far_clip, inout int shadowmap_idx, inout mat3x4 M, inout int cascade) {
	float z_next = unproject_depth(depth_next_tile);
	float z_start = unproject_depth(depth);
	float z_center = mix(z_start, z_next, .5f);
	float thickness = z_start - z_next;

	// Scatter
	vec3 scattered;
	if (ld.type == LightTypeDirectional) {
		// For directional lights, first update cascade if needed
		if (-z_start >= current_cascade_far_clip) {
			++cascade;
			current_cascade_far_clip = cascades_depths[cascade];
			shadowmap_idx = light_get_cascade_shadowmap_idx(ld, cascade);
			M = light_cascade_projection(cascade_descriptor, 
										 cascade, 
										 ld.transformed_position,
										 cascades_depths);
		}

		scattered = scatter_directional_light(slice_coords,
											  depth,
											  z_start,
											  z_next,
											  thickness,
											  light_idx,
											  ld,
											  M,
											  shadowmap_idx,
											  min_lum);
	}
	else {
		scattered = scatter_spherical_light(slice_coords,
											depth,
											z_start,
											z_next,
											thickness,
											light_idx,
											ld,
											ll_idx,
											min_lum);
	}

	return scattered;
}

void main() {
	// Read work coordinates
	ivec3 volume_size = imageSize(volume);
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);
	if (any(greaterThanEqual(slice_coords, volume_size.xy)))
		return;

	// Query depth of geometry at current work coordinates and limit end tile respectively
	int depth_lod = lll_depth_lod;
	float depth_buffer_d_min, depth_buffer_d_max;
	depth_limits_3x3((vec2(slice_coords) + vec2(.5f)) / vec2(volume_size.xy), depth_lod, depth_buffer_d_min, depth_buffer_d_max);

	int effective_tiles_start = max(int(floor(volumetric_scattering_tile_for_depth(depth_buffer_d_max))), 0);
	int effective_tiles_end = min(int(ceil(volumetric_scattering_tile_for_depth(depth_buffer_d_min))) + 2, volumetric_scattering_depth_tiles);
	
	vec2 fragcoords = slice_coords_to_fragcoords(vec2(slice_coords));
	vec2 next_tile_fragcoords = slice_coords_to_fragcoords(vec2(slice_coords + ivec2(1)));

	// Loop through per-pixel linked-light-list
	uint32_t lll_start = imageLoad(lll_heads, slice_coords).x;
	uint32_t lll_length = imageLoad(lll_size, slice_coords).x;
	for (uint32_t lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
			
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		uint ll_idx = uint(lll_parse_ll_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];

		// We use the low detail variation of the per-pixel linked-light-list,
		// therefore we have a different minimal light luminance than usual lights.
		float min_lum = lll_low_detail_light_minimal_luminance(ld);
		
		// Cascade data used for directional lights
		uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		float current_cascade_far_clip = .0f;
		int cascade = 0;
		int shadowmap_idx;
		mat3x4 M;
		
		// Compute tight limits on tiles to sample based on pp-lll depth ranges
		int tiles_effected_by_light_start = int(floor(volumetric_scattering_tile_for_depth(lll_depth_range.y))); 
		int tiles_effected_by_light_end = int(ceil(volumetric_scattering_tile_for_depth(lll_depth_range.x))); 

		// Iterate over the tiles and accumulate up to the effective tiles
		vec3 accum = vec3(.0f);
		int tile = tiles_effected_by_light_start;
		float depth = volumetric_scattering_depth_for_tile(tile);
		for (; tile < effective_tiles_end; ++tile) {
			// For each tile, generate tile information and scatter
			ivec3 volume_coords = ivec3(slice_coords, tile);
			float depth_next_tile = volumetric_scattering_depth_for_tile(tile + 1);

			if (tile < tiles_effected_by_light_end) {
				accum += scatter(depth, depth_next_tile, 
								 slice_coords, fragcoords, next_tile_fragcoords,
								 ld, light_idx, ll_idx, min_lum,
								 cascade_descriptor, current_cascade_far_clip, shadowmap_idx, M, cascade);
			}

			vec3 stored_rgb = imageLoad(volume, volume_coords).rgb;
			imageStore(volume, volume_coords, vec4(stored_rgb + accum, .0f));

			// Next depth
			depth = depth_next_tile;
		}
	}
}
