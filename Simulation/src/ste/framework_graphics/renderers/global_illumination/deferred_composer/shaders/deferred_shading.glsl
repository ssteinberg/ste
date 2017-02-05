
#include "common.glsl"
#include "deferred_shading_common.glsl"

#include "chromaticity.glsl"
#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "intersection.glsl"

#include "atmospherics.glsl"
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

float deferred_evaluate_shadowing(deferred_shading_shadow_maps shadow_maps, 
								  fragment_shading_parameters frag,
								  light_shading_parameters light,
								  int cascade) {
	float l_radius = light.ld.radius;

	if (light_type_is_directional(light.ld.type)) {
		// Query cascade index, and shadowmap index and construct cascade projection matrix
		uint cascade_idx = light_get_cascade_descriptor_idx(light.ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		int shadowmap_idx = light_get_cascade_shadowmap_idx(light.ld, cascade);

		// Construct matrix to transform into cascade-space
		vec2 cascade_recp_vp;
		float cascade_proj_far;
		mat3x4 M = light_cascade_projection(cascade_descriptor, 
											cascade, 
											light.ld.transformed_position,
											cascades_depths,
											cascade_recp_vp,
											cascade_proj_far);

		return shadow(shadow_maps,
					  shadowmap_idx,
					  frag.p,
					  frag.n,
					  M,
					  cascade_recp_vp,
					  cascade_proj_far,
					  light.l_dist,
					  l_radius,
					  frag.coords);
	}
	else {
		vec3 shadow_v = frag.world_position - light.ld.position;
		return shadow(shadow_maps,
					  light.ll_id,
					  frag.p,
					  frag.n,
					  shadow_v,
					  l_radius,
					  light.ld.effective_range,
					  frag.coords);
	}
}

vec3 deferred_shade_atmospheric_scattering(ivec2 coord, deferred_atmospherics_luts atmospherics_luts) {
	vec3 position = unproject_screen_position(.5f, vec2(coord) / vec2(backbuffer_size()));
	vec3 w_pos = transform_view_to_world_space(position);

	vec3 P = eye_position();
	vec3 V = normalize(w_pos - P);

	vec3 rgb = vec3(.0f);
	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint lll_start = imageLoad(lll_heads, lll_coords).x;
	uint lll_length = imageLoad(lll_size, lll_coords).x;
	for (uint lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];
		
		if (light_type_is_directional(ld.type)) {
			vec3 L = ld.position;
			vec3 I0 = irradiance(ld, .0f);

			rgb += I0 * atmospheric_scatter(P, L, V, 
											atmospherics_luts.atmospheric_scattering_lut,
											atmospherics_luts.atmospheric_mie0_scattering_lut);

			//? Draw the light source.
			//!? TODO: Remove in future.
			vec3 light_position = P - L * ld.directional_distance;
			if (!isinf(intersection_ray_sphere(light_position, ld.radius,
											   P, V))) {
				rgb += I0 * extinct_ray(P, V,
										atmospherics_luts.atmospheric_optical_length_lut);
			}
		}
	}

	return rgb;
}

vec3 deferred_compute_attenuation_from_fragment_to_eye(fragment_shading_parameters frag,
													   deferred_atmospherics_luts atmospherics_luts) {
	return extinct(eye_position(), frag.world_position,
				   atmospherics_luts.atmospheric_optical_length_lut);
}

bool deferred_generate_light_shading_parameters(fragment_shading_parameters frag,
												light_descriptor ld,
												uint light_id, uint ll_id,
												deferred_atmospherics_luts atmospherics_luts,
												out light_shading_parameters light) {
	light.ld = ld;
	light.light_id = light_id;
	light.ll_id = ll_id;

	vec3 cd_m2;										// Light illuminance reaching fragment
	vec3 l = light_incidant_ray(ld, frag.p);		// Light incident ray
	if (light_type_is_directional(ld.type)) {
		light.l_dist = abs(ld.directional_distance);
		
		// Atmopsheric attenuation
		vec3 atat = extinct_ray(frag.world_position, -ld.position,
								atmospherics_luts.atmospheric_optical_length_lut);

		cd_m2 = irradiance(ld) * atat;
		
		//! Atmospheric ambient light (TODO: Ambient occlusion)
		cd_m2 += atmospheric_ambient(frag.world_position, dot(frag.n, -ld.transformed_position), ld.position,
								   atmospherics_luts.atmospheric_ambient_lut);
	}
	else {
		float dist2 = dot(l, l);
		if (dist2 >= sqr(ld.effective_range)) {
			// Bail out
			light.cd_m2 = vec3(.0f);
			return false;
		}
		
		// Atmopsheric attenuation
		vec3 atat = extinct(ld.position, frag.world_position,
							atmospherics_luts.atmospheric_optical_length_lut);

		light.l_dist = sqrt(dist2);
		l /= light.l_dist;

		cd_m2 = irradiance(ld) * atat;
	}

	light.l = l;
	light.cd_m2 = cd_m2;

	return true;
}


bool fragment_facing_light_source(fragment_shading_parameters frag, 
								  light_shading_parameters light) {
	float N_dot_L = dot(frag.n, light.l);
	if (!light_type_is_shaped(light.ld.type)) {
		if (N_dot_L <= .0f)
			return false;
	}
	else {
		if (N_dot_L > .0f)
			return true;

		float tan_theta = light.ld.radius / light.l_dist;
		float tan_theta2 = sqr(tan_theta);
		float sin_theta2 = tan_theta2 / (1.f + tan_theta2);
		if (sqr(N_dot_L) >= sin_theta2)
			return false;
	}

	return true;
}

vec3 deferred_shade_fragment(g_buffer_element gbuffer_frag, ivec2 coord,
							 deferred_shading_shadow_maps shadow_maps,
							 deferred_material_microfacet_luts material_microfacet_luts, 
							 deferred_material_ltc_luts ltc_luts,
							 sampler3D scattering_volume, 
							 deferred_atmospherics_luts atmospherics_luts,
							 sampler2D back_face_depth, 
							 sampler2D front_face_depth) {
	vec3 accum_luminance = vec3(.0f);
	fragment_shading_parameters frag;

	// Calculate perceived object thickness in camera space (used for subsurface scattering)
	bool has_geometry;
	float thickness = get_thickness(coord, back_face_depth, front_face_depth, has_geometry);
	
	// If no geometry is present, calculate atmopsheric scattering and that's it
	if (!has_geometry) {
		return deferred_shade_atmospheric_scattering(coord, atmospherics_luts);
	}

	// Read gbuffer fragment information
	gbuffer_fragment_information frag_info = gbuffer_parse_fragment_information(gbuffer_frag);

	// Extrapolate view position and world position
	float depth = frag_info.depth;
	frag.p = unproject_screen_position(depth, vec2(coord) / vec2(backbuffer_size()));
	frag.world_position = transform_view_to_world_space(frag.p);

	// Load material
	material_descriptor md = mat_descriptor[frag_info.mat];
	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	vec3 material_texture = material_base_texture(md, frag_info.uv, frag_info.duvdx, frag_info.duvdy).rgb;
	float cavity = material_cavity(md, frag_info.uv, frag_info.duvdx, frag_info.duvdy);
	bool material_has_sss = material_has_subsurface_scattering(md);

	// Normal map
	frag.n = frag_info.n;
	frag.t = frag_info.t;
	frag.b = frag_info.b;
	normal_map(md, frag_info.uv, frag_info.duvdx, frag_info.duvdy, frag.n, frag.t, frag.b);

	// Fill in the rest of shaded fragment properties
	frag.coords = coord;
	frag.v = normalize(-frag.p);
	frag.world_v = normalize(eye_position() - frag.world_position);
	frag.world_normal = transform_direction_view_to_world_space(frag.n);

	// Atmospheric attenuation from eye to fragment
	vec3 atmospheric_attenuation = deferred_compute_attenuation_from_fragment_to_eye(frag, atmospherics_luts);

	// Directional light cascade
	int cascade = light_which_cascade_for_position(frag.p, cascades_depths);
	
	// Add material emission
	accum_luminance += material_emission(md);

	// Iterate lights in the linked-light-list structure
	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint lll_start = imageLoad(lll_heads, lll_coords).x;
	uint lll_length = imageLoad(lll_size, lll_coords).x;
	for (uint lll_ptr = lll_start; lll_ptr != lll_start + lll_length; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];

		// Check that light is in depth range
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth < lll_depth_range.x) 
			continue;
			
		// Translate light id from linked-light-list to light-buffer index and load light data
		uint light_idx = uint(lll_parse_light_idx(lll_p));
		uint ll_idx = uint(lll_parse_ll_idx(lll_p));
		light_descriptor ld = light_buffer[light_idx];

		// Compute light properties, bail if fragment is unaffected by light
		light_shading_parameters light;
		if (!deferred_generate_light_shading_parameters(frag,
														ld, light_idx, ll_idx,
														atmospherics_luts,
														light))
			continue;

		// For simple materials, bail if fragment is not facing light
		if (!material_has_sss && !fragment_facing_light_source(frag, light))
			continue;

		// Shadow query
		float shdw = deferred_evaluate_shadowing(shadow_maps,
												 frag,
												 light,
												 cascade);
		float occlusion = max(.0f, cavity * shdw);

		// For simple materials, bail is fully shadowed
		if (!material_has_sss && occlusion == .0f)
			continue;
			
		// Evaluate material luminance for given light
		vec3 luminance = material_evaluate_radiance(md,
													head_layer,
													frag,
													light,
													thickness,
													material_microfacet_luts,
													ltc_luts,
													shadow_maps, 
													occlusion);
		accum_luminance += material_texture.rgb * luminance;
	}

	// Volumetric scattered light
	// Volumetric scattering has atmospheric attenuation precomputed
	vec3 scattered_incoming_luminance = vec3(0);//volumetric_scattering(scattering_volume, vec2(coord), depth);

	// Apply atmospheric attenuation
	vec3 final = accum_luminance * atmospheric_attenuation + scattered_incoming_luminance;

	return final;
}
