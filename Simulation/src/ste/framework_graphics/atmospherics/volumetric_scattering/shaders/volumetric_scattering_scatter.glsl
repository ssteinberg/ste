
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

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

layout(bindless_sampler) uniform sampler2D downsampled_depth_map;
layout(bindless_sampler) uniform sampler2D depth_map;

uniform float cascades_depths[directional_light_cascades];

const float samples = 2.f;

void depth_limits_3x3(vec2 uv, int lod, out float mind, out float maxd) {
	vec2 d00 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2(-1,-1)).xy;
	vec2 d10 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2( 0,-1)).xy;
	vec2 d20 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2( 1,-1)).xy;
	vec2 d01 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2(-1, 0)).xy;
	vec2 d11 = textureLod(downsampled_depth_map, uv, lod).xy;
	vec2 d21 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2( 1, 0)).xy;
	vec2 d02 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2(-1, 1)).xy;
	vec2 d12 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2( 0, 1)).xy;
	vec2 d22 = textureLodOffset(downsampled_depth_map, uv, lod, ivec2( 1, 1)).xy;
	
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

vec3 scatter_spherical_light(vec2 slice_coords,
							 vec3 position,
							 vec3 w_pos,
							 float thickness,
							 light_descriptor ld,
							 uint shadowmap_idx,
							 float min_lum) {
	vec3 l = light_incidant_ray(ld, position);
	float l_dist = length(l);
	l /= l_dist;
					
	vec3 shadow_v = w_pos - ld.position;
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
							   ld.radius,
							   light_effective_range(ld));
	if (shadow <= .0f)
		return vec3(0);

	return irradiance(ld, l_dist) * scatter(ld.position, w_pos, eye_position(),
											thickness,
											atmospheric_optical_length_lut);
}

vec3 scatter_directional_light(vec2 slice_coords,
							   vec3 position,
							   vec3 w_pos,
							   float thickness,
							   light_descriptor ld,
							   mat3x4 M,
							   float cascade_proj_far,
							   int shadowmap_idx,
							   float min_lum) {
	vec3 l = light_incidant_ray(ld, position);					
	float shadow = shadow_test(directional_shadow_depth_maps,
							   shadowmap_idx,
							   position,
							   M,
							   cascade_proj_far);
	if (shadow <= .0f)
		return vec3(0);
			
	return irradiance(ld, .0f) * scatter_ray(eye_position(), w_pos, -ld.position,
											 thickness,
											 atmospheric_optical_length_lut);
}

bool generate_sample(vec2 slice_coords, float s, 
					 float depth, float depth_next_tile, float z0, float z2,
					 uint light_idx,
					 out vec3 position, out vec3 w_pos) {
	// Generate a seed
	vec2 seed = seed_scattering(slice_coords, light_idx, s, depth);

	// Generate a random number, and use it to generate a pseudo-random sampling point
	float r = fast_rand(seed);

	// Use some noise to generate 2 more pseudo-random numbers from r. Use them to generate the xy sampling coordinates.
	vec2 noise = vec2(65198.10937f, 22109.532971f);
	vec2 jitter = fract(r * noise);
	vec2 coords = slice_coords_to_fragcoords(vec2(slice_coords) + jitter);

	// Check depth buffer
	float d = texture(depth_map, coords).x;
	//if (d > depth)
	//	return false;
	d = min(d, depth);

	float next_d = max(d, depth_next_tile);
	float z1 = unproject_depth(next_d);

	float z = mix(z0, z1, r);

	position = unproject_screen_position_with_z(z, coords);
	w_pos = transform_view_to_world_space(position);

	return true;
}

vec3 scatter(float depth, float depth_next_tile, 
			 ivec2 slice_coords, vec2 fragcoords, vec2 next_tile_fragcoords,
			 light_descriptor ld, uint light_idx, uint ll_idx, float min_lum,
			 light_cascade_descriptor cascade_descriptor, inout float current_cascade_far_clip, inout int shadowmap_idx, inout mat3x4 M, inout float cascade_proj_far, inout int cascade) {
	float z0 = unproject_depth(depth);
	float z2 = unproject_depth(depth_next_tile);
	float thickness = abs(z0 - z2);

	vec3 position, w_pos;

	// Scatter
	vec3 scattered = vec3(0);
	if (light_type_is_directional(ld.type)) {
		// For directional lights, first update cascade if needed
		if (-z0 >= current_cascade_far_clip) {
			++cascade;
			current_cascade_far_clip = cascades_depths[cascade];
			shadowmap_idx = light_get_cascade_shadowmap_idx(ld, cascade);
			M = light_cascade_projection(cascade_descriptor, 
										 cascade, 
										 ld.transformed_position,
										 cascades_depths,
										 cascade_proj_far);
		}
		
		for (float s = 0; s < samples; ++s) {
			generate_sample(slice_coords, s, 
							depth, depth_next_tile, z0, z2,
							light_idx,
							position, w_pos);

			scattered += scatter_directional_light(slice_coords,
												   position,
												   w_pos,
												   thickness,
												   ld,
												   M,
												   cascade_proj_far,
												   shadowmap_idx,
												   min_lum);
		}
	}
	else {
		for (float s = 0; s < samples; ++s) {
			generate_sample(slice_coords, s, 
							depth, depth_next_tile, z0, z2,
							light_idx,
							position, w_pos);

			scattered += scatter_spherical_light(slice_coords,
												 position,
												 w_pos,
												 thickness,
												 ld,
												 ll_idx,
												 min_lum);
		}
	}

	return scattered / samples;
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

	float effective_tiles_start = max(volumetric_scattering_tile_for_depth(depth_buffer_d_max) - 2.f, 0.f);
	float effective_tiles_end = min(volumetric_scattering_tile_for_depth(depth_buffer_d_min) + 1.f, float(volumetric_scattering_depth_tiles));
	
	vec2 fragcoords = slice_coords_to_fragcoords(vec2(slice_coords));
	vec2 next_tile_fragcoords = slice_coords_to_fragcoords(vec2(slice_coords + ivec2(1)));

	// Loop through per-pixel linked-light-list
	uint lll_start = imageLoad(lll_heads, slice_coords).x;
	uint lll_length = imageLoad(lll_size, slice_coords).x;
	for (uint lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
			
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		uint ll_idx = uint(lll_parse_ll_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];

		// We use the low detail variation of the per-pixel linked-light-list,
		// therefore we have a different minimal light luminance than usual lights.
		float min_lum = lll_low_detail_light_minimal_luminance(ld);
		
		// Cascade data used for directional lights
		uint cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		float current_cascade_far_clip = .0f, cascade_proj_far;
		int cascade = 0;
		int shadowmap_idx;
		mat3x4 M;
		
		// Compute tight limits on tiles to sample based on pp-lll depth ranges
		float tiles_effected_by_light_start = volumetric_scattering_tile_for_depth(lll_depth_range.y);
		float tiles_effected_by_light_end = volumetric_scattering_tile_for_depth(lll_depth_range.x);
	
		// Iterate over the tiles and accumulate up to the effective tiles
		vec3 accum = vec3(.0f);
		float tile_start = floor(tiles_effected_by_light_start);
		float tile = tile_start;
		float depth = volumetric_scattering_depth_for_tile(tile);
		for (; tile <= effective_tiles_end; ++tile) {
			// For each tile, generate tile information and scatter
			ivec3 volume_coords = ivec3(slice_coords, int(tile + .5f));
			float depth_next_tile = volumetric_scattering_depth_for_tile(tile + 1.f);

			if (tile <= tiles_effected_by_light_end)
				accum += scatter(depth, depth_next_tile, 
								 slice_coords, fragcoords, next_tile_fragcoords,
								 ld, light_idx, ll_idx, min_lum,
								 cascade_descriptor, current_cascade_far_clip, shadowmap_idx, M, cascade_proj_far, cascade);

			vec3 stored_rgb = imageLoad(volume, volume_coords).rgb;
			imageStore(volume, volume_coords, vec4(stored_rgb + accum, .0f));

			// Next depth
			depth = depth_next_tile;
		}
	}
}
