
#include "material.glsl"

vec3 material_evaluate_layer_radiance(material_layer_descriptor layer,
									  vec3 n,
									  vec3 t,
									  vec3 b,
									  vec3 v,
									  vec3 l,
									  vec2 uv,
									  vec2 duvdx,
									  vec2 duvdy,
									  float F0,
									  vec3 irradiance,
									  out float D,
									  out float G,
									  out float F) {
	vec3 h = normalize(v + l);

	// Read layer parameters
	float roughness = layer.roughness;
	float metallic = layer.metallic;
	float anisotropy_ratio = layer.anisotropy_ratio;
	float sheen = layer.sheen_ratio;
	float sheen_power = layer.sheen_power;
	
	// Colors and tints
	vec3 base_color = material_layer_base_color(layer, uv, duvdx, duvdy);
	vec3 specular_tint = vec3(1);
	vec3 sheen_tint = vec3(1);

	// Specular color
	vec3 c_spec = mix(specular_tint, base_color, metallic);

	// Additional sheen color (diffuse at grazing angles)
	vec3 c_sheen = fresnel_schlick_ratio(dot(l,h), sheen_power) * sheen_tint * sheen;

	// Specular
	vec3 Specular;
	if (anisotropy_ratio != 1.f) {
		float roughness_x = roughness * anisotropy_ratio;
		float roughness_y = roughness / anisotropy_ratio;

		Specular = cook_torrance_ansi_brdf(n, t, b,
										   v, l, h,
										   roughness_x,
										   roughness_y,
										   F0, c_spec,
										   D, G, F);
	} else {
		Specular = cook_torrance_iso_brdf(n, v, l, h,
										  roughness,
										  F0, c_spec,
										  D, G, F);
	}

	// Diffuse
	vec3 Diffuse = base_color * disney_diffuse_brdf(n, v, l, h,
													roughness);

	// Evaluate BRDF
	vec3 brdf = Specular + (1.f - metallic) * (Diffuse + c_sheen);
	return brdf * irradiance * dot(n, l);
}

bool material_snell_refraction(inout vec3 v,
							   vec3 n,
							   float ior1,
							   float ior2) {
	vec3 t = cross(n, -v);
	float ior = ior1 / ior2;

	float cosine2 = 1.f - ior * ior * dot(t, t);
	if (cosine2 < .0f)
		return false;

	vec3 normal_by_cosine = n * sqrt(cosine2);
	v = normal_by_cosine - ior * cross(n, -t);

	return true;
}

vec3 material_evaluate_radiance(material_layer_descriptor layer,
								vec3 n,
								vec3 t,
								vec3 b,
								vec3 v,
								vec3 l,
								vec2 uv,
								vec2 duvdx,
								vec2 duvdy,
								vec3 irradiance,
								float external_medium_ior = 1.00029f) {
	vec3 rgb = vec3(0);
	
	float F0 = material_convert_ior_to_F0(external_medium_ior, layer.ior);
	vec3 atten = vec3(1.f);
	while (true) {
		float D;
		float G;
		float F;
		rgb += atten * material_evaluate_layer_radiance(layer,
														n, t, b,
														v, l,
														uv, duvdx, duvdy,
														F0,
														irradiance, 
														D, G, F);

		uint32_t next_layer_id = layer.next_layer_id;
		if (next_layer_id == material_none)
			break;

		material_layer_descriptor next_layer = mat_layer_descriptor[next_layer_id];
	
		if (!material_snell_refraction(v, n, layer.ior, next_layer.ior) ||
			!material_snell_refraction(l, n, layer.ior, next_layer.ior))
			break;
	
		vec3 absorption_coefficient = vec3(.1f);
		float thickness = layer.thickness;
		float path_length = thickness * (1.f / dot(v, n) + 1.f / dot(l, n));
		vec3 absorption = exp(-path_length * absorption_coefficient);
	
		vec3 h = normalize(v + l);
		float F21 = fresnel_schlick(F0, dot(l,h));
	
		float T12 = 1.f - F;
		float T21 = 1.f - F21;
		float g = (1.f - G) + T21 * G;

		atten *= absorption * T12 * g;
		F0 = material_convert_ior_to_F0(layer.ior, next_layer.ior);
		layer = next_layer;
	}

	return rgb;
}
