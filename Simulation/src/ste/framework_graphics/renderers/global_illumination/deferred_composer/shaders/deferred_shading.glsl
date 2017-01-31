
#include "common.glsl"
#include "deferred_shading_common.glsl"

#include "chromaticity.glsl"
#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "intersection.glsl"

#include "atmospherics.glsl"
#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

#include "linearly_transformed_cosines.glsl"

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

	if (light.ld.type == LightTypeDirectional) {
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
		
		if (ld.type == LightTypeDirectional) {
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
												deferred_material_ltc_luts ltc_luts,
												out light_shading_parameters light) {
	light.ld = ld;
	light.light_id = light_id;
	light.ll_id = ll_id;

	vec3 lux;										// Light illuminance reaching fragment
	vec3 l = light_incidant_ray(ld, frag.p);		// Light incident ray
	if (ld.type == LightTypeDirectional) {
		light.l_dist = abs(ld.directional_distance);
		
		// Atmopsheric attenuation
		vec3 atat = extinct_ray(frag.world_position, -ld.position,
								atmospherics_luts.atmospheric_optical_length_lut);

		lux = irradiance(ld, 0) * atat;
		
		//! Atmospheric ambient light (TODO: Ambient occlusion)
		lux += atmospheric_ambient(frag.world_position, dot(frag.n, -ld.transformed_position), ld.position,
								   atmospherics_luts.atmospheric_ambient_lut);
	}
	else {
		float dist2 = dot(l, l);
		if (dist2 >= sqr(ld.effective_range)) {
			// Bail out
			light.lux = vec3(.0f);
			return false;
		}
		
		// Atmopsheric attenuation
		vec3 atat = extinct(ld.position, frag.world_position,
							atmospherics_luts.atmospheric_optical_length_lut);

		light.l_dist = sqrt(dist2);
		l /= light.l_dist;

		if (light_id == 0) {
			vec3 u_quadPoints[4] = {
				transform_view(vec3(-700.6, 138, 100)),
				transform_view(vec3(-600.6, 138, -120)),
				transform_view(vec3(-600.6, 438, -120)),
				transform_view(vec3(-700.6, 438, 100)),
			};

			vec2 coords = LTC_Coords(dot(frag.n, frag.o), .25f);
			mat3 Minv   = LTC_Matrix(ltc_luts.ltc_ggx_fit, coords);
			vec3 Lo_i   = LTC_Evaluate(frag.n, frag.o, frag.p, Minv, u_quadPoints, true);

			lux = irradiance(ld, light.l_dist) * atat * Lo_i;
		}else
		lux = vec3(0);

		//lux = irradiance(ld, light.l_dist) * atat;
	}

	light.l = l;
	light.lux = lux;

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
	vec3 rgb = vec3(.0f);
	fragment_shading_parameters frag;

	// Calculate perceived object thickness in camera space (used for subsurface scattering)
	bool has_geometry;
	float thickness = get_thickness(coord, back_face_depth, front_face_depth, has_geometry);
	
	// If no geometry is present, calculate atmopsheric scattering and that's it
	if (!has_geometry) {
		return deferred_shade_atmospheric_scattering(coord, atmospherics_luts);
	}

	// Calculate depth and extrapolate world position
	float depth = gbuffer_parse_depth(gbuffer_frag);
	frag.p = unproject_screen_position(depth, vec2(coord) / vec2(backbuffer_size()));
	frag.world_position = transform_view_to_world_space(frag.p);

	// Read G-buffer data from fragment 
	int draw_idx = gbuffer_parse_material(gbuffer_frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(gbuffer_frag);
	vec2 duvdx = gbuffer_parse_duvdx(gbuffer_frag);
	vec2 duvdy = gbuffer_parse_duvdy(gbuffer_frag);

	// Normal map
	frag.n = gbuffer_parse_normal(gbuffer_frag);
	frag.t = gbuffer_parse_tangent(gbuffer_frag);
	frag.b = cross(frag.t, frag.n);
	normal_map(md, uv, duvdx, duvdy, frag.n, frag.t, frag.b);

	// Fill in the rest of shaded fragment properties
	frag.coords = coord;
	frag.o = normalize(-frag.p);

	// Read material data
	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	vec3 material_texture = material_base_texture(md, uv, duvdx, duvdy).rgb;
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	// Atmospheric attenuation from eye to fragment
	vec3 atmospheric_attenuation = deferred_compute_attenuation_from_fragment_to_eye(frag, atmospherics_luts);

	// Directional light cascade
	int cascade = light_which_cascade_for_position(frag.p, cascades_depths);

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

		// Compute light properties
		light_shading_parameters light;
		if (!deferred_generate_light_shading_parameters(frag,
														ld, light_idx, ll_idx,
														atmospherics_luts,
														ltc_luts,
														light))
			continue;

		// Shadow query
		float shdw = deferred_evaluate_shadowing(shadow_maps,
												 frag,
												 light,
												 cascade);

		if (ld.type == LightTypeDirectional) {
			//!? TODO: Remove!
			// Inject some ambient, still without global illumination...
			rgb += ld.diffuse * ld.luminance * 1e-11 * (1-shdw);
		}

		float occlusion = max(.0f, cavity * shdw);
			
		// Evaluate material luminance for given light
		vec3 luminance = material_evaluate_radiance(head_layer,
													frag,
													light,
													thickness,
													material_microfacet_luts,
													shadow_maps, 
													occlusion);
		rgb += material_texture.rgb * luminance;
	}

	// Add material emission
	rgb += material_emission(md);

	// Apply atmospheric attenuation
	rgb *= atmospheric_attenuation;

	// Apply volumetric scattered light to computed radiance
	// Volumetric scattering has atmospheric attenuation precomputed
	rgb += volumetric_scattering(scattering_volume, vec2(coord), depth);

	return rgb;
}
