
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

#include "volumetric_scattering.glsl"
#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"
#include "girenderer_matrix_buffer.glsl"

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

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"

uniform float proj00, proj11, proj22, proj23, shadow_proj22, shadow_proj23;

vec3 unproject_position(float depth, vec2 frag_xy) {
	vec3 frag_coords = vec3(frag_xy, depth);
	vec3 ndc = (frag_coords - vec3(.5f)) * 2.f;

	float z = (proj23 / (ndc.z + proj22));
	vec2 xy = (ndc.xy * z) / vec2(proj00, proj11);

	return vec3(xy, -z);
}

void main() {
	mat4 inverse_view_matrix = transpose(view_matrix_buffer.transpose_inverse_view_matrix);

	ivec3 volume_coords = ivec3(gl_GlobalInvocationID.xyz);
	vec2 fragcoords = vec2(volume_coords.xy) / vec2(1500.f /8.f, 1500.f /16.f*9.f/8.f);

	float depth = volumetric_scattering_depth_for_tile(volume_coords.z);
	float depth_next_tile = volumetric_scattering_depth_for_tile(volume_coords.z + 1);
	vec3 position = unproject_position(depth, fragcoords);
	float z_next_tile = unproject_position(depth_next_tile, fragcoords).z;

	float thickness = position.z - z_next_tile;
	vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;
	vec3 view_dir = normalize(position);

	float particle_density = volumetric_scattering_particle_density(w_pos);
	float k_scattering = volumetric_scattering_scattering_coefficient(particle_density, thickness);
	float k_absorption = volumetric_scattering_absorption_coefficient(particle_density, thickness);
	vec3 fog_diffuse = vec3(1.f);

	vec3 rgb = vec3(.0f);

	ivec2 lll_coords = volume_coords.xy;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
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
			float shadow = shadow_penumbra_width(shadow_depth_maps,
												 uint(lll_parse_ll_idx(lll_p)),
												 shadow_v,
												 l_radius,
												 dist,
												 shadow_proj22,
												 shadow_proj23);
			if (shadow >= 1.f)
				continue;

			float attenuation_factor = light_attenuation_factor(ld, dist);
			float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);
			float irradiance = incident_radiance * (1.f - shadow);

			rgb += ld.diffuse * volumetric_scattering_phase(v / dist, view_dir);
		}
	}

	rgb *= fog_diffuse;

	imageStore(volume, volume_coords, vec4(rgb * k_scattering, k_scattering + k_absorption));
}
