
#include "chromaticity.glsl"

#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

float get_thickness(ivec2 coord,
					sampler2D back_face_depth, 
					sampler2D front_face_depth) {
	float fd = unproject_depth(texelFetch(front_face_depth, coord, 0).x);
	float bd = unproject_depth(texelFetch(back_face_depth, coord, 0).x);

	return fd - bd;
}

vec3 deferred_shade_fragment(g_buffer_element frag, ivec2 coord,
							 samplerCubeArrayShadow shadow_depth_maps, 
							 samplerCubeArray shadow_maps, 
							 sampler3D scattering_volume, 
							 sampler2D back_face_depth, 
							 sampler2D front_face_depth) {
	int draw_idx = gbuffer_parse_material(frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	float depth = gbuffer_parse_depth(frag);
	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(backbuffer_size()));
	vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, position);

	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, uv, duvdx, duvdy, n, t, b);

	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	float thickness = get_thickness(coord, back_face_depth, front_face_depth);

	vec3 rgb = material_emission(md);

	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;

		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x) {
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			uint light_id = uint(lll_parse_ll_idx(lll_p));

			vec3 incident = light_incidant_ray(ld, position);
			if (dot(n, incident) <= 0)
				continue;

			float light_effective_range = ld.effective_range;
			float dist2 = dot(incident, incident);
			if (dist2 >= light_effective_range*light_effective_range)
				continue;

			float l_radius = ld.radius;
			vec3 shadow_v = w_pos - ld.position;
			float shdw = shadow(shadow_depth_maps,
								shadow_maps,
								light_id,
								shadow_v,
								l_radius);
			if (shdw <= .0f)
				continue;

			float dist = sqrt(dist2);

			vec3 v = normalize(-position);
			vec3 l = incident / dist;
			vec3 irradiance = light_irradiance(ld, dist) * cavity * shdw;
			
			rgb += material_evaluate_radiance(head_layer,
											  position,
											  n, t, b,
											  v, l,
											  uv, duvdx, duvdy,
											  thickness,
											  ld,
											  shadow_maps, light_id,
											  irradiance);
		}
	}

	rgb = volumetric_scattering(scattering_volume, rgb, vec2(gl_FragCoord.xy), depth);

	return rgb;
}
