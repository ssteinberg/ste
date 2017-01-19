
#include "chromaticity.glsl"

#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "intersection.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

float get_thickness(ivec2 coord,
					sampler2D back_face_depth, 
					sampler2D front_face_depth,
					out bool has_geometry) {
	float fd = texelFetch(front_face_depth, coord, 0).x;
	float fz = unproject_depth(fd);
	float bz = unproject_depth(texelFetch(back_face_depth, coord, 0).x);

	has_geometry = fd > .0f;

	return fz - bz;
}

float deferred_evaluate_shadowing(samplerCubeArrayShadow shadow_depth_maps, 
								  samplerCubeArray shadow_maps, 
								  sampler2DArrayShadow directional_shadow_depth_maps,
								  sampler2DArray directional_shadow_maps, 
								  int cascade,
								  vec3 position,
								  vec3 world_position,
								  vec3 normal,
								  uint light_id,
								  float l_dist,
								  light_descriptor ld, 
								  ivec2 coord) {
	float l_radius = ld.radius;

	if (ld.type == LightTypeDirectional) {
		// Query cascade index, and shadowmap index and construct cascade projection matrix
		uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		int shadowmap_idx = light_get_cascade_shadowmap_idx(ld, cascade);

		// Construct matrix to transform into cascade-space
		vec2 cascade_recp_vp;
		mat3x4 M = light_cascade_projection(cascade_descriptor, 
											cascade, 
											ld.transformed_position,
											cascades_depths,
											cascade_recp_vp);

		return shadow(directional_shadow_depth_maps,
					  directional_shadow_maps,
					  shadowmap_idx,
					  position,
					  normal,
					  M,
					  cascade_recp_vp,
					  l_dist,
					  l_radius,
					  coord);
	}
	else {
		vec3 shadow_v = world_position - ld.position;
		return shadow(shadow_depth_maps,
					  shadow_maps,
					  light_id,
					  position,
					  normal,
					  shadow_v,
					  l_radius,
					  coord);
	}
}

vec3 deferred_shade_atmospheric_scattering(ivec2 coord, 
										   sampler2DArray atmospheric_optical_length_lut,
										   sampler3D atmospheric_scattering_lut,
										   sampler3D atmospheric_mie0_scattering_lut) {
	vec3 position = unproject_screen_position(.5f, vec2(coord) / vec2(backbuffer_size()));
	vec3 w_pos = transform_view_to_world_space(position);

	vec3 P = eye_position();
	vec3 V = normalize(w_pos - P);

	if (V.y < .0) return vec3(0);

	vec3 rgb = vec3(.0f);
	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint32_t lll_start = imageLoad(lll_heads, lll_coords).x;
	uint32_t lll_length = imageLoad(lll_size, lll_coords).x;
	for (uint32_t lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];
		
		if (ld.type == LightTypeDirectional) {
			vec3 L = ld.position;
			vec3 I0 = irradiance(ld, .0f);

			rgb += I0 * atmospheric_scatter(P, L, V, 
											atmospheric_scattering_lut,
											atmospheric_mie0_scattering_lut);

			//? Draw the light source.
			//!? TODO: Remove in future.
			vec3 light_position = P - L * ld.directional_distance;
			/*if (!isinf(intersection_ray_sphere(light_position, ld.radius,
											   P, V))) {
				rgb += I0 * extinct_ray(P, V,
										atmospheric_optical_length_lut);
			}*/
		}
	}

	return rgb;
}

vec3 deferred_shade_fragment(g_buffer_element frag, ivec2 coord,
							 samplerCubeArrayShadow shadow_depth_maps, 
							 samplerCubeArray shadow_maps, 
							 sampler2DArrayShadow directional_shadow_depth_maps,
							 sampler2DArray directional_shadow_maps,
							 sampler3D scattering_volume, 
							 sampler2D microfacet_refraction_fit_lut, 
							 sampler2DArray microfacet_transmission_fit_lut, 
							 sampler2DArray atmospheric_optical_length_lut,
							 sampler3D atmospheric_scattering_lut,
							 sampler3D atmospheric_mie0_scattering_lut,
							 sampler2D back_face_depth, 
							 sampler2D front_face_depth) {
	// Calculate perceived object thickness in camera space (used for subsurface scattering)
	bool has_geometry;
	float thickness = get_thickness(coord, back_face_depth, front_face_depth, has_geometry);
	
	// If no geometry is present, calculate atmopsheric scattering and that's it
	//if (!has_geometry) {
		return deferred_shade_atmospheric_scattering(coord,
													 atmospheric_optical_length_lut,
													 atmospheric_scattering_lut,
													 atmospheric_mie0_scattering_lut);
	//}

	// Calculate depth and extrapolate world position
	float depth = gbuffer_parse_depth(frag);
	vec3 position = unproject_screen_position(depth, vec2(coord) / vec2(backbuffer_size()));
	vec3 w_pos = transform_view_to_world_space(position);

	// Read G-buffer data from fragment 
	int draw_idx = gbuffer_parse_material(frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	// Normal map
	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, uv, duvdx, duvdy, n, t, b);

	// Read material data
	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	vec4 material_texture = material_base_texture(md, uv, duvdx, duvdy);
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	// Directional light cascade
	int cascade = light_which_cascade_for_position(position, cascades_depths);

	// Iterate lights in the linked-light-list structure
	vec3 rgb = vec3(.0f);
	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint32_t lll_start = imageLoad(lll_heads, lll_coords).x;
	uint32_t lll_length = imageLoad(lll_size, lll_coords).x;
	for (uint32_t lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];

		// Check that light is in depth range
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x) {
			// Translate light id from linked-light-list to light-buffer index and load light data
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			// Light id is used for shadow map access
			uint light_id = uint(lll_parse_ll_idx(lll_p));
			
			// Atmospheric extinction
			vec3 atat;
			// Compute light incident ray and range
			float l_dist;
			vec3 l = light_incidant_ray(ld, position);
			if (ld.type == LightTypeDirectional) {
				atat = extinct_ray(eye_position(), w_pos, -ld.position,
								   atmospheric_optical_length_lut);
				l_dist = abs(ld.directional_distance);
			}
			else {
				atat = extinct(ld.position, w_pos, eye_position(),
							   atmospheric_optical_length_lut);

				float light_effective_range = ld.effective_range;
				float dist2 = dot(l, l);
				if (dist2 >= light_effective_range*light_effective_range)
					continue;

				l_dist = sqrt(dist2);
				l /= l_dist;
			}

			// Shadow query
			float shdw = deferred_evaluate_shadowing(shadow_depth_maps, 
													 shadow_maps, 
													 directional_shadow_depth_maps,
													 directional_shadow_maps,
													 cascade,
													 position,
													 w_pos,
													 n,
													 light_id,
													 l_dist,
													 ld,
													 coord);

			if (ld.type == LightTypeDirectional) {
				//!? TODO: Remove!
				// Inject some ambient, still without global illumination...
				rgb += ld.diffuse * ld.luminance * 1e-11 * (1-shdw);
			}

			// Calculate occlusion, distance to light, normalized incident and reflection (eye) vectors
			float occlusion = max(.0f, cavity * shdw);
			vec3 v = normalize(-position);
			
			// Evaluate material radiance for given light
			rgb += atat * material_texture.rgb * material_evaluate_radiance(head_layer,
																			position,
																			n, t, b,
																			v, l,
																			thickness,
																			ld,
																			microfacet_refraction_fit_lut,
																			microfacet_transmission_fit_lut,
																			shadow_maps, light_id,
																			l_dist,
																			occlusion,
																			coord);
		}
	}

	// Add material emission
	rgb += material_emission(md);

	// Apply volumetric lighting to computed radiance
	rgb = volumetric_scattering(scattering_volume, rgb, vec2(coord), depth);

	return rgb;
}
