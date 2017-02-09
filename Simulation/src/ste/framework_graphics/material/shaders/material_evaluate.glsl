
#include "material.glsl"
#include "material_layer_unpack.glsl"

#include "subsurface_scattering.glsl"

#include "deferred_shading_common.glsl"
#include "common.glsl"

#include "light.glsl"
#include "light_transport.glsl"

#include "cosine_distribution_integration.glsl"
#include "clamped_cosine_distribution_integration.glsl"
#include "microfacet_ggx_fitting.glsl"
#include "cook_torrance.glsl"
#include "lambert_diffuse.glsl"
#include "disney_diffuse.glsl"
#include "fresnel.glsl"

vec3 material_evaluate_layer_radiance(material_layer_unpacked_descriptor descriptor,
									  light_descriptor ld,
									  float l_dist,
									  vec3 wp,
									  vec3 wn,
									  vec3 wv,
									  vec3 n, vec3 t, vec3 b,
									  vec3 v, vec3 l, vec3 h,
									  float cos_critical, 
									  float refractive_ratio,
									  vec3 irradiance,
									  vec3 albedo,
									  vec3 diffused_light,
									  deferred_material_ltc_luts ltc_luts) {	
	// Specular color
	vec3 specular_tint = vec3(1);
	vec3 c_spec = mix(specular_tint, albedo, descriptor.metallic);

	// Shading type
	bool is_directional = light_type_is_directional(ld.type);
	bool ltc_integration = light_type_is_shaped(ld.type) || is_directional;

	if (ltc_integration) {
		// Calculate polygonal light irradiance using linearly transformed cosines
		vec2 ltccoords = ltc_lut_coords(ltc_luts.ltc_ggx_fit, dot(n, v), descriptor.roughness);
		mat3 ltc_M_inv = ltc_inv_matrix(ltc_luts.ltc_ggx_fit, ltccoords);
		float ltc_ampl = texture(ltc_luts.ltc_ggx_amplitude, ltccoords).x;
		
		// Read light shape
		bool shape_sphere = light_shape_is_sphere(ld.type);
		bool shape_quad = light_shape_is_quad(ld.type);
		bool shape_polygon = light_shape_is_polygon(ld.type);
		bool shape_polyhedron = light_shape_is_convex_polyhedron(ld.type);
		
		// And properties
		bool two_sided = light_type_is_two_sided(ld.type);
		bool textured = light_type_is_textured(ld.type);

		// Points count and offset into points buffer
		uint points_count = light_get_polygon_point_counts(ld);
		uint points_offset = light_get_polygon_point_offset(ld);
		
		// Light position
		vec3 L = is_directional ? 
					wp + l * ld.directional_distance : 
					ld.position;
		vec3 wl = (L - wp) / l_dist;
	
		// The integration type depends on shape
		vec3 specular_irradiance;
		vec3 diffuse_irradiance;
		if (shape_sphere || is_directional) {
			// No points needed for spherical light
			// Directional lights are (very far) sphere lights. Treat them identically.
			float r = ld.radius;

			specular_irradiance = ltc_evaluate_sphere(wn, wv, wp, ltc_M_inv, L, r) * ltc_ampl;
			diffuse_irradiance  = integrate_cosine_distribution_sphere_cross_section(l_dist, r).xxx;//ltc_evaluate_sphere(wn, wv, wp, mat3(1),   L, r);
		}
		else if (shape_quad) {
			// Quad. Always 4 points.
			specular_irradiance = ltc_evaluate_quad(wn, wv, wp, ltc_M_inv, L, points_offset, two_sided) * ltc_ampl;
			diffuse_irradiance  = shaped_light_attenuation(ld, l_dist, wl).xxx;//ltc_evaluate_quad(wn, wv, wp, mat3(1),   L, points_offset, two_sided);
		}
		else if (shape_polygon) {
			specular_irradiance = ltc_evaluate_polygon(wn, wv, wp, ltc_M_inv, L, points_count, points_offset, two_sided) * ltc_ampl;
			diffuse_irradiance  = shaped_light_attenuation(ld, l_dist, wl).xxx;//ltc_evaluate_polygon(wn, wv, wp, mat3(1),   L, points_count, points_offset, two_sided);
		}
		else /*if (shape_polyhedron)*/ {
			// Polyhedron light. Primitives are always triangles.
			uint primitives = points_count / 3;

			specular_irradiance = ltc_evaluate_convex_polyhedron(wn, wv, wp, ltc_M_inv, L, primitives, points_offset) * ltc_ampl;
			diffuse_irradiance  = ltc_evaluate_convex_polyhedron(wn, wv, wp, mat3(1),   L, primitives, points_offset);
		}
		
		// Compute fresnel term
		float F = fresnel(dot(l, h), cos_critical, refractive_ratio);
		
		// And finalize
		vec3 Specular = F * c_spec * specular_irradiance;
		vec3 Diffuse = diffused_light * diffuse_irradiance;

		return irradiance * (Specular + Diffuse);
	}
	else {	// Virtual light
		// For non-integrated lights we need to factor light attenuation manually.
		float attenuation = virtual_light_attenuation(ld, l_dist);

		// Anisotropic roughness
		float rx = descriptor.roughness * descriptor.anisotropy_ratio;
		float ry = descriptor.roughness / descriptor.anisotropy_ratio;

		// Evaluate BRDFs
		vec3 Specular = cook_torrance_ansi_brdf(n, t, b,  
												v, l, h, 
												rx, ry, 
												cos_critical,  
												refractive_ratio, 
												c_spec);
		vec3 Diffuse = diffused_light * lambert_diffuse_brdf();
		
		vec3 brdf = irradiance * dot(n, l) * (Specular + Diffuse);
		return max(vec3(.0f), attenuation * brdf);
	}
}

float material_attenuation_through_layer(float transmittance,
										 float metallic,
										 float masking) {
	float passthrough = 1.f - metallic;
	return transmittance * masking * passthrough;
}

/*
 *	Evaluate radiance of material at fragment. Simple single-layered materials without subsurface scattering.
 *
 *	@param layer		Material layer
 *	@param frag			Fragment shading parameters
 *	@param light		Light shading parameters
 *	@param material_microfacet_luts	Microfacet GGX fitting LUTs
 *	@param occlusion	Light occlusion
 *	@param external_medium_ior	Index-of-refraction of source medium
 */
vec3 material_evaluate_radiance_simple(material_layer_unpacked_descriptor descriptor,
									   fragment_shading_parameters frag,
									   light_shading_parameters light,
									   deferred_material_microfacet_luts material_microfacet_luts,
									   deferred_material_ltc_luts ltc_luts, 
									   float occlusion,
									   float external_medium_ior = 1.0002772f) {		
	// Compute sine and cosine of critical angle
	float refractive_ratio = descriptor.ior / external_medium_ior;
	float cos_critical = refractive_ratio < 1.f ? 
							sqrt(1.f - refractive_ratio * refractive_ratio) :
							.0f;

	// Evaluate refracted vectors
	/*vec3 refracted_v = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
									frag.v, frag.n,
									descriptor.roughness,
									refractive_ratio);
	vec3 refracted_l = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
									light.l, frag.n,
									descriptor.roughness,
									refractive_ratio);*/

	// Evaluate total inner (downwards into material) and outer (upwards towards eye) transmission
	float inner_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																frag.v, frag.n, 
																descriptor.roughness, 
																refractive_ratio);
	float outer_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																/*refracted_l*/light.l, frag.n, 
																descriptor.roughness, 
																1.f / refractive_ratio);

	vec3 scattering = inner_transmission_ratio * outer_transmission_ratio * descriptor.albedo.rgb;
	vec3 diffused_light = descriptor.albedo.rgb * (1.f - descriptor.metallic);

	// Half vector
	vec3 h = normalize(frag.v + light.l);
	// Evaluate layer BRDF
	vec3 rgb = material_evaluate_layer_radiance(descriptor,
												light.ld,
												light.l_dist,
												frag.world_position,
												frag.world_normal,
												frag.world_v, 
												frag.n, frag.t, frag.b,
												frag.v, light.l, h,
												cos_critical, 
												refractive_ratio,
												light.cd_m2,
												descriptor.albedo.rgb,
												diffused_light,
												ltc_luts);

	return rgb * occlusion;
}

/*
 *	Evaluate radiance of material at fragment
 *
 *	@param md			Material descriptor
 *	@param layer		Material layer
 *	@param frag			Fragment shading parameters
 *	@param light		Light shading parameters
 *	@param object_thickness	Object thickness at shaded fragment
 *	@param material_microfacet_luts	Microfacet GGX fitting LUTs
 *	@param shadow_maps	Shadow maps
 *	@param occlusion	Light occlusion
 *	@param external_medium_ior	Index-of-refraction of source medium
 */
vec3 material_evaluate_radiance(material_descriptor md,
								material_layer_descriptor layer,
								fragment_shading_parameters frag,
								light_shading_parameters light,
								float object_thickness,
								deferred_material_microfacet_luts material_microfacet_luts,
								deferred_material_ltc_luts ltc_luts, 
								deferred_shading_shadow_maps shadow_maps,
								float occlusion,
								float external_medium_ior = 1.0002772f) {
	material_layer_unpacked_descriptor descriptor = material_layer_unpack(layer);

	// A simple material is a material without subsurface scattering and a single layer.
	// Use a faster codepath in that case.
	bool simple_material = material_is_simple(md, layer);
	if (simple_material) {
		return material_evaluate_radiance_simple(descriptor,
												 frag,
												 light,
												 material_microfacet_luts,
												 ltc_luts,
												 occlusion,
												 external_medium_ior);
	}

	vec3 rgb = vec3(0);

	vec3 n = frag.n;
	vec3 t = frag.t;
	vec3 b = frag.b;
	vec3 position = frag.p;
	ivec2 frag_coords = frag.coords;
		
	vec3 l = light.l;
	vec3 v = frag.v;
	float top_medium_ior = external_medium_ior;

	// Attenuation at current layer
	vec3 attenuation = vec3(1.f);
	// Attenuation of outgoing radiance, for sub-surface scattering
	vec3 sss_attenuation = vec3(1.f);

	while (true) {
		// Read layer properties
		vec3 albedo = descriptor.albedo.rgb;
		float thickness = material_is_base_layer(descriptor) ? object_thickness : descriptor.thickness;
		float metallic = descriptor.metallic;
		float roughness = descriptor.roughness;
		vec3 attenuation_coefficient = descriptor.attenuation_coefficient;
		float bottom_medium_ior = descriptor.ior;
		
		// Compute sine and cosine of critical angle
		float refractive_ratio = bottom_medium_ior / top_medium_ior;
		float cos_critical = refractive_ratio < 1.f ? 
								sqrt(1.f - refractive_ratio * refractive_ratio) :
								.0f;
		// Compute Fresnel reflection at parallel incidence
		float F0 = fresnel_F0(refractive_ratio);

		// Evaluate refracted vectors
		/*vec3 refracted_v = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
										v, n,
										roughness,
										refractive_ratio);
		vec3 refracted_l = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
										l, n,
										roughness,
										refractive_ratio);*/

		// Evaluate total inner (downwards into material) and outer (upwards towards eye) transmission
		float inner_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																   v, n, 
																   roughness, 
																   refractive_ratio);
		float outer_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																   /*refracted_l*/l, n, 
																   roughness, 
																   1.f / refractive_ratio);
	
		// Compute total and outer path lengths inside current layer
		//float dotNV = max(epsilon, dot(n,refracted_v));
		//float dotNL = max(epsilon, dot(n,refracted_l));
		float dotNV = max(epsilon, dot(n,v));
		float dotNL = max(epsilon, dot(n,l));
		float path_length = thickness * (1.f / dotNV + 1.f / dotNL);
		float outer_path_length = thickness / dotNL;
		
		// Compute light attenuation in layer
		vec3 k = beer_lambert(attenuation_coefficient, path_length);
		// Compute light attenuation in layer on outgoing direction only
		vec3 sss_outer_extinction = beer_lambert(attenuation_coefficient, outer_path_length);

		// Diffused light is a portion of the energy scattered inside layer (based on albedo), unattenuated light doesn't contribute to diffuse
		vec3 scattering = inner_transmission_ratio * outer_transmission_ratio * (vec3(1.f) - k) * albedo;
		vec3 diffused_light = scattering * (1.f - metallic);

		// Half vector
		vec3 h = normalize(v + l);
		// Evaluate layer BRDF
		rgb += attenuation * material_evaluate_layer_radiance(descriptor,
															  light.ld,
															  light.l_dist,
															  frag.world_position,
															  frag.world_normal,
															  frag.world_v, 
															  n, t, b,
															  v, l, h,
															  cos_critical, 
															  refractive_ratio,
															  light.cd_m2,
															  albedo,
															  diffused_light,
															  ltc_luts);
							
		// Update incident and outgoing vectors to refracted ones before continuing to next layer
		//v = refracted_v;
		//l = refracted_l;
	
		// Compute attenuated light due to Fresnel transmittance, microfacet masking-shadowing and metallicity
		float layer_surface_outer_attenuation = material_attenuation_through_layer(outer_transmission_ratio, 
																				   metallic,
																				   1/*Gmask*/);

		// For sub-surface scattering we assume normal incidence of light, and we attenuate based on (presumed) incident and outgoing sides
		sss_attenuation *= layer_surface_outer_attenuation * material_attenuation_through_layer(1.f - F0, metallic, 1.f);

		// If this is the base layer, stop
		if (material_is_base_layer(descriptor))
			break;

		// Otherwise, update attenuation with attenuation at layer surface and with attenuated (absorbed and scattered) light
					
		float layer_surface_inner_attenuation = material_attenuation_through_layer(inner_transmission_ratio, 
																				   metallic,
																				   1/*Gshadow*/);
		float total_layer_surface_attenuation = layer_surface_inner_attenuation * layer_surface_outer_attenuation;

		attenuation *= total_layer_surface_attenuation * k;
		// Update sss attenuation with attenuation on the way out and attenuation on the way in with parallel incident
		sss_attenuation *= sss_outer_extinction * beer_lambert(attenuation_coefficient, thickness);

		// Set ior and descriptor for next layer
		top_medium_ior = bottom_medium_ior;
		descriptor = material_layer_unpack(mat_layer_descriptor[descriptor.next_layer_id]);
	}

	// Apply occlusion
	rgb *= occlusion;
	
	// Sub-surface scattering
	bool fully_attenuated = all(lessThan(sss_attenuation, vec3(epsilon)));
	if (!fully_attenuated) {
		rgb += sss_attenuation * subsurface_scattering(descriptor,
													   position,
													   n,
							 						   object_thickness,
													   light.ld,
													   shadow_maps, 
													   light,
													   -v,
													   frag_coords);
	}

	return rgb;
}
