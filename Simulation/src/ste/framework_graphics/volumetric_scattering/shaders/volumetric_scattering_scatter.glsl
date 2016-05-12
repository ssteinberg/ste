
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

#include "volumetric_scattering.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"

#include "girenderer_matrix_buffer.glsl"
#include "project.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};
layout(shared, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(shared, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

layout(rgba16f, binding = 7) restrict writeonly uniform image3D volume;

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(binding = 11) uniform sampler2D depth_map;

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"

uniform float proj00, proj11, proj23, shadow_proj23;
uniform vec2 backbuffer_size;

float depth3x3(ivec2 uv, int lod) {
	float d00 = texelFetchOffset(depth_map, uv, lod, ivec2(-1,-1)).x;
	float d10 = texelFetchOffset(depth_map, uv, lod, ivec2( 0,-1)).x;
	float d20 = texelFetchOffset(depth_map, uv, lod, ivec2( 1,-1)).x;
	float d01 = texelFetchOffset(depth_map, uv, lod, ivec2(-1, 0)).x;
	float d11 = texelFetch(depth_map, uv, lod).x;
	float d21 = texelFetchOffset(depth_map, uv, lod, ivec2( 1, 0)).x;
	float d02 = texelFetchOffset(depth_map, uv, lod, ivec2(-1, 1)).x;
	float d12 = texelFetchOffset(depth_map, uv, lod, ivec2( 0, 1)).x;
	float d22 = texelFetchOffset(depth_map, uv, lod, ivec2( 1, 1)).x;

	float a = min(min(d00, d10), min(d20, d01));
	float b = min(min(d11, d21), min(d02, d12));
	float c = min(a, b);
	return min(c, d22);
}

void main() {
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);
	if (slice_coords.x >= imageSize(volume).x ||
		slice_coords.y >= imageSize(volume).y)
		return;

	int depth_lod = 2;
	vec2 fragcoords = (vec2(slice_coords) + vec2(.5f)) * 8.f / backbuffer_size;
	mat4 inverse_view_matrix = transpose(view_matrix_buffer.transpose_inverse_view_matrix);

	float depth_buffer_d = depth3x3(slice_coords, depth_lod);

	uint32_t lll_ptr_base = imageLoad(lll_heads, slice_coords).x;

	int max_tile = min(int(ceil(volumetric_scattering_tile_for_depth(depth_buffer_d))) + 2, volumetric_scattering_depth_tiles);
	float depth = volumetric_scattering_depth_for_tile(0);
	for (int tile = 0; tile < max_tile; ++tile) {
		ivec3 volume_coords = ivec3(slice_coords, tile);

		float depth_next_tile = volumetric_scattering_depth_for_tile(tile + 1);
		vec3 position = unproject_screen_position(depth, fragcoords, proj23, proj00, proj11);
		float z_next_tile = unproject_depth(depth_next_tile, proj23);

		float thickness = position.z - z_next_tile;
		vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;
		vec3 view_dir = normalize(position);

		float particle_density = volumetric_scattering_particle_density(w_pos);
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
			if (depth >= lll_depth_range.x &&
				depth <= lll_depth_range.y) {
				uint light_idx = uint(lll_parse_light_idx(lll_p));
				light_descriptor ld = light_buffer[light_idx];

				vec3 v = light_incidant_ray(ld, light_idx, position);

				float dist = length(v);
				float l_radius = ld.radius;

				vec3 shadow_v = w_pos - ld.position_direction.xyz;
				float shadow = shadow(shadow_depth_maps,
									uint(lll_parse_ll_idx(lll_p)),
									shadow_v,
									l_radius,
									dist,
									shadow_proj23);
				if (shadow <= .0f)
					continue;

				float attenuation_factor = light_attenuation_factor(ld, dist);
				float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);
				float irradiance = incident_radiance * shadow;

				rgb += ld.diffuse * volumetric_scattering_phase(v / dist, view_dir);
			}
		}

		imageStore(volume, volume_coords, vec4(rgb * fog_diffuse * k_scattering, k_scattering + k_absorption));

		depth = depth_next_tile;
	}
}
