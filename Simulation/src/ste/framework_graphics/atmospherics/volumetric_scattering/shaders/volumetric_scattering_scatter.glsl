
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

layout(bindless_sampler) uniform sampler2D depth_map;

uniform float cascades_depths[directional_light_cascades];

const int samples = 2;

float depth3x3(vec2 uv, int lod) {
	float d00 = textureLodOffset(depth_map, uv, lod, ivec2(-1,-1)).x;
	float d10 = textureLodOffset(depth_map, uv, lod, ivec2( 0,-1)).x;
	float d20 = textureLodOffset(depth_map, uv, lod, ivec2( 1,-1)).x;
	float d01 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 0)).x;
	float d11 = textureLod(depth_map, uv, lod).x;
	float d21 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 0)).x;
	float d02 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 1)).x;
	float d12 = textureLodOffset(depth_map, uv, lod, ivec2( 0, 1)).x;
	float d22 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 1)).x;

	float a = min(min(d00, d10), min(d20, d01));
	float b = min(min(d11, d21), min(d02, d12));
	float c = min(a, b);
	return min(c, d22);
}

vec2 slice_coords_to_fragcoords(vec2 v) {
	return (v + vec2(.5f)) * 8.f / vec2(backbuffer_size());
}

float calculate_tile_volume(float z, float thickness, vec2 fragcoords, vec2 next_tile_fragcoords) {
	vec2 eye_space_xy = unproject_screen_position_with_z(z, fragcoords).xy;
	vec2 eye_space_next_xy = unproject_screen_position_with_z(z, next_tile_fragcoords).xy;
	vec2 l = eye_space_next_xy - eye_space_xy;
	return thickness * l.x * l.y;
}

vec3 calculate_scattering_position(vec2 seed, vec2 slice_coords, float z_start, float z_next) {
	float r = fast_rand(seed);
	float z = mix(z_start, z_next, r * .9999f);

	vec2 noise = vec2(65198.0937f, -22109.32971f);
	vec2 jitter = fract(r * noise);
	vec2 coords = slice_coords_to_fragcoords(vec2(slice_coords) + jitter);
	return unproject_screen_position_with_z(z, coords);
}

vec3 calculate_scatter(vec3 position, float tile_volume, light_descriptor ld, vec3 l, float l_dist, float shadow) {
	float position_len = length(position);
	vec3 view_dir = -position / position_len;
	return scatter(ld, 
				   position, 
				   tile_volume, 
				   l, l_dist, 
				   view_dir, position_len) * shadow;
}

vec3 scatter_spherical_light(vec2 slice_coords,
							 float tile_volume,
							 float depth,
							 float z_start,
							 float z_next,
							 float thickness,
							 uint light_idx,
							 light_descriptor ld,
							 uint shadowmap_idx) {
	vec3 rgb = vec3(.0f);
	for (int s = 0; s < samples; ++s) {
		vec3 position = calculate_scattering_position(slice_coords + vec2(light_idx + s * .1f, depth),
													  slice_coords,
													  z_start, z_next);
				
		vec3 l = light_incidant_ray(ld, position);
		float l_dist = length(l);
		l /= l_dist;
					
		vec3 shadow_v = position - ld.transformed_position;
		float shadow = shadow_fast(shadow_depth_maps,
								   shadowmap_idx,
								   shadow_v,
								   ld.radius);

		float scaling_size = thickness;
		float scale = min(l_dist, scaling_size) / scaling_size;

		if (shadow <= .0f)
			continue;

		rgb += calculate_scatter(position, tile_volume, ld, l, l_dist, shadow) * scale;
	}

	return rgb;
}

vec3 scatter_directional_light(vec2 slice_coords,
							   float tile_volume,
							   float depth,
							   float z_start,
							   float z_next,
							   uint light_idx,
							   light_descriptor ld,
							   mat3x4 M,
							   int shadowmap_idx) {
	vec3 rgb = vec3(.0f);
	for (int s = 0; s < samples; ++s) {
		vec3 position = calculate_scattering_position(slice_coords + vec2(light_idx + s, depth),
													  slice_coords,
													  z_start, z_next);
				
		vec3 l = light_incidant_ray(ld, position);					
		float shadow = shadow_fast(directional_shadow_depth_maps,
								   shadowmap_idx,
								   position,
								   M);
		if (shadow <= .0f)
			continue;
			
		rgb += calculate_scatter(position, tile_volume, ld, l, .0f, shadow);
	}

	return rgb;
}

void main() {
	// Read work coordinates
	ivec3 volume_size = imageSize(volume);
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);
	if (any(greaterThanEqual(slice_coords, volume_size.xy)))
		return;

	// Query depth of geometry at current work coordinates and limit end tile respectively
	int depth_lod = 2;
	float depth_buffer_d = depth3x3((vec2(slice_coords) + vec2(.5f)) / vec2(volume_size.xy), depth_lod);
	int max_tile = min(int(ceil(volumetric_scattering_tile_for_depth(depth_buffer_d))) + 2, volumetric_scattering_depth_tiles);
	
	vec2 fragcoords = slice_coords_to_fragcoords(vec2(slice_coords));
	vec2 next_tile_fragcoords = slice_coords_to_fragcoords(vec2(slice_coords + ivec2(1)));

	// Loop through per-pixel linked-light-list
	uint32_t lll_ptr = imageLoad(lll_heads, slice_coords).x;
	for (;;++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;
			
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];
		
		// Cascade data used for directional lights
		uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		float current_cascade_far_clip = .0f;
		int cascade = 0;
		int shadowmap_idx;
		mat3x4 M;
		
		// Compute tight limits on tiles to sample based on pp-lll depth ranges
		int tile = int(floor(volumetric_scattering_tile_for_depth(lll_depth_range.y)));
		int end_tile = min(int(ceil(volumetric_scattering_tile_for_depth(lll_depth_range.x))), max_tile);
		float depth = volumetric_scattering_depth_for_tile(tile);
		for (; tile < end_tile; ++tile) {
			// For each tile, generate tile information and read atmospheric data
			ivec3 volume_coords = ivec3(slice_coords, tile);
			float depth_next_tile = volumetric_scattering_depth_for_tile(tile + 1);

			float z_next = unproject_depth(depth_next_tile);
			float z_start = unproject_depth(depth);
			float z_center = mix(z_start, z_next, .5f);
			float thickness = z_start - z_next;

			// Calculate tile volume
			float tile_volume = calculate_tile_volume(z_center, thickness, fragcoords, next_tile_fragcoords);

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
													  tile_volume,
													  depth,
													  z_start,
													  z_next,
													  light_idx,
													  ld,
													  M,
													  shadowmap_idx);
			}
			else {
				scattered = scatter_spherical_light(slice_coords,
													tile_volume,
													depth,
													z_start,
													z_next,
													thickness,
													light_idx,
													ld,
													uint(lll_parse_ll_idx(lll_p)));
			}

			// Read current tile value and update
			vec3 rgb = scattered / float(samples);
			rgb += imageLoad(volume, volume_coords).rgb;
			imageStore(volume, volume_coords, vec4(rgb, .0f));

			// Next depth
			depth = depth_next_tile;
		}
	}
}
