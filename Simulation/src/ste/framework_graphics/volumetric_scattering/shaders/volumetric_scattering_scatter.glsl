
#type compute
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

#include "volumetric_scattering.glsl"

#include "light_transport.glsl"
#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "girenderer_transform_buffer.glsl"
#include "project.glsl"

#include "interleaved_gradient_noise.glsl"

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

layout(rgba16f, binding = 7) restrict writeonly uniform image3D volume;

#include "linked_light_lists_load.glsl"

layout(bindless_sampler) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(bindless_sampler) uniform sampler2DArrayShadow directional_shadow_depth_maps;

layout(bindless_sampler) uniform sampler2D depth_map;

uniform float phase;
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

void main() {
	ivec3 volume_size = imageSize(volume);
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);
	if (slice_coords.x >= volume_size.x ||
		slice_coords.y >= volume_size.y)
		return;

	int depth_lod = 2;
	float depth_buffer_d = depth3x3((vec2(slice_coords) + vec2(.5f)) / vec2(volume_size.xy), depth_lod);

	uint32_t lll_ptr_base = imageLoad(lll_heads, slice_coords).x;

	int max_tile = min(int(ceil(volumetric_scattering_tile_for_depth(depth_buffer_d))) + 2, volumetric_scattering_depth_tiles);
	float depth = volumetric_scattering_depth_for_tile(0);
	for (int tile = 0; tile < max_tile; ++tile) {
		ivec3 volume_coords = ivec3(slice_coords, tile);
		vec2 fragcoords = slice_coords_to_fragcoords(vec2(slice_coords));

		float depth_next_tile = volumetric_scattering_depth_for_tile(tile + 1);
		float z_next = unproject_depth(depth_next_tile);
		float z_start = unproject_depth(depth);
		float z_center = mix(z_start, z_next, .5f);
		float thickness = z_start - z_next;

		float particle_density = volumetric_scattering_particle_density(unproject_screen_position_with_z(z_center, fragcoords));
		float k_scattering = volumetric_scattering_scattering_coefficient(particle_density, thickness);
		float k_absorption = volumetric_scattering_absorption_coefficient(particle_density, thickness);
		vec3 fog_diffuse = vec3(1.f);

		vec3 rgb = vec3(.0f);

		uint32_t lll_ptr = lll_ptr_base;
		for (;;++lll_ptr) {
			lll_element lll_p = lll_buffer[lll_ptr];
			if (lll_eof(lll_p))
				break;

			vec2 lll_depth_range = lll_parse_depth_range(lll_p);
			if (depth_next_tile >= lll_depth_range.x &&
				depth <= lll_depth_range.y) {
				uint light_idx = uint(lll_parse_light_idx(lll_p));
				light_descriptor ld = light_buffer[light_idx];

				vec3 scatter = vec3(0.f);
				for (int s = 0; s < samples; ++s) {
					float r = interleaved_gradient_noise(slice_coords + vec2(light_idx + s + 1, depth));
					float z = mix(z_start, z_next, r * .99f);
					
					vec2 jitter = vec2(fract(r * 1.696f), fract(-r * 2.329f));
					vec2 coords = slice_coords_to_fragcoords(vec2(slice_coords) + jitter);
					vec3 position = unproject_screen_position_with_z(z, coords);

					float l_dist;
					vec3 l;
					vec3 incident = light_incidant_ray(ld, position);
					if (ld.type == LightTypeSphere) {
						float light_effective_range = ld.effective_range;
						float dist2 = dot(incident, incident);
						if (dist2 >= light_effective_range*light_effective_range)
							continue;

						l_dist = sqrt(dist2);
						l = incident / l_dist;
					}
					else {
						l_dist = abs(ld.directional_distance);
						l = incident;
					}

					float shadow = 1.f;
					if (ld.type == LightTypeSphere) {
						vec3 shadow_v = position - ld.transformed_position;
						shadow = shadow_fast(shadow_depth_maps,
											 uint(lll_parse_ll_idx(lll_p)),
											 shadow_v,
											 ld.radius);
					}
					else {
						// Query cascade index, and shadowmap index and construct cascade projection matrix
						uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
						light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];

						int cascade = light_which_cascade_for_position(position, cascades_depths);
						int shadowmap_idx = light_get_cascade_shadowmap_idx(ld, cascade);
						
						// Construct matrix to transform into cascade-space
						mat3x4 M = light_cascade_projection(cascade_descriptor, 
															cascade, 
															ld.transformed_position,
															cascades_depths);

						shadow = shadow_fast(directional_shadow_depth_maps,
											 shadowmap_idx,
											 position,
											 M);
					}
					if (shadow <= .0f)
						continue;

					vec3 view_dir = normalize(position);

					vec3 irradiance = light_irradiance(ld, l_dist) * shadow;

					float scaling_size = thickness;
					float scale = min(l_dist, scaling_size) / scaling_size;

					scatter += scale * irradiance * henyey_greenstein_phase_function(l, view_dir, phase);
				}

				rgb += scatter / float(samples);
			}
		}

		imageStore(volume, volume_coords, vec4(rgb * fog_diffuse * k_scattering, k_scattering + k_absorption));

		depth = depth_next_tile;
	}
}
