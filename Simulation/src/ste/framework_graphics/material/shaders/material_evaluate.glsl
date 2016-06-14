
#include "material.glsl"
#include "material_layer_unpack.glsl"

vec3 material_evaluate_layer_radiance(material_layer_descriptor layer,
									  vec3 n,
									  vec3 t,
									  vec3 b,
									  vec3 v,
									  vec3 l,
									  material_layer_unpacked_descriptor descriptor,
									  float F0,
									  vec3 irradiance,
									  out float D,
									  out float G,
									  out float F) {
	vec3 h = normalize(v + l);
	vec3 base_color = descriptor.base_color;
	
	// Tints
	vec3 specular_tint = vec3(1);
	vec3 sheen_tint = vec3(1);

	// Specular color
	vec3 c_spec = mix(specular_tint, base_color, descriptor.metallic);

	// Additional sheen color (diffuse at grazing angles)
	vec3 c_sheen = fresnel_schlick_ratio(dot(l,h), descriptor.sheen_power) * sheen_tint * descriptor.sheen;

	// Specular
	vec3 Specular;
	if (descriptor.anisotropy_ratio != 1.f) {
		float roughness_x = descriptor.roughness * descriptor.anisotropy_ratio;
		float roughness_y = descriptor.roughness / descriptor.anisotropy_ratio;

		Specular = cook_torrance_ansi_brdf(n, t, b,
										   v, l, h,
										   roughness_x,
										   roughness_y,
										   F0, c_spec,
										   D, G, F);
	} else {
		Specular = cook_torrance_iso_brdf(n, v, l, h,
										  descriptor.roughness,
										  F0, c_spec,
										  D, G, F);
	}

	// Diffuse
	vec3 Diffuse = base_color * disney_diffuse_brdf(n, v, l, h,
													descriptor.roughness);

	// Evaluate BRDF
	vec3 brdf = Specular + (1.f - descriptor.metallic) * (Diffuse + c_sheen);
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
		material_layer_unpacked_descriptor descriptor = material_layer_unpack(layer, uv, duvdx, duvdy);

		float D;
		float G;
		float F;
		rgb += atten * material_evaluate_layer_radiance(layer,
														n, t, b,
														v, l,
														descriptor,
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
			
		float thickness = descriptor.thickness;
		float metallic = descriptor.metallic;
	
		vec3 absorption_coefficient = vec3(.1f);
		float path_length = thickness * (1.f / dot(v, n) + 1.f / dot(l, n));
		vec3 absorption = exp(-path_length * absorption_coefficient);
	
		vec3 h = normalize(v + l);
		float F21 = fresnel_schlick(F0, dot(l,h));
	
		float T12 = 1.f - F;
		float T21 = 1.f - F21;
		float g = (1.f - G) + T21 * G;
		float passthrough = 1.f - metallic;

		atten *= absorption * T12 * g * passthrough;
		F0 = material_convert_ior_to_F0(layer.ior, next_layer.ior);
		layer = next_layer;
	}

	return rgb;
}
