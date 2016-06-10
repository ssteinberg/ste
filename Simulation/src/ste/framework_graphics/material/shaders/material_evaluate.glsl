
#include "material.glsl"

vec3 material_evaluate_irradiance(uint32_t head_layer,
								  vec3 n,
								  vec3 t,
								  vec3 b,
								  vec3 v,
								  vec3 l,
								  vec2 uv,
								  vec2 duvdx,
								  vec2 duvdy,
								  vec3 irradiance) {
	material_layer_descriptor layer = mat_layer_descriptor[head_layer];

	vec3 h = normalize(v + l);

	// Read layer parameters
	float roughness = layer.roughness;
	float metallic = layer.metallic;
	float F0 = material_convert_ior_to_F0(layer.ior, 1.f);
	float anisotropy_ratio = layer.anisotropy_ratio;
	float sheen = layer.sheen_ratio;
	float sheen_power = layer.sheen_power;
	
	// Colors and tints
	vec3 base_color = material_layer_base_color(layer, uv, duvdx, duvdy);
	vec3 specular_tint = vec3(1);
	vec3 sheen_tint = vec3(1);

	// Specular color
	vec3 c_spec = F0 * mix(specular_tint, base_color, metallic);

	// Additional sheen color (diffuse at grazing angles)
	vec3 c_sheen = fresnel_schlick(dot(l,h), sheen_power) * sheen_tint * sheen;

	// Specular
	vec3 S;
	if (anisotropy_ratio != 1.f) {
		float roughness_x = roughness * anisotropy_ratio;
		float roughness_y = roughness / anisotropy_ratio;

		S = cook_torrance_ansi_brdf(n, t, b,
									v, l, h,
									roughness_x,
									roughness_y,
									c_spec);
	} else {
		S = cook_torrance_iso_brdf(n, v, l, h,
								   roughness,
								   c_spec);
	}

	// Diffuse
	vec3 D = base_color * disney_diffuse_brdf(n, v, l, h,
											  roughness);

	// Evaluate BRDF
	vec3 brdf = S + (1.f - metallic) * (D + c_sheen);
	return brdf * irradiance * dot(n, l);
}
