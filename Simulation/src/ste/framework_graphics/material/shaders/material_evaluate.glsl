
#include "material.glsl"

void normal_map(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b, inout vec3 P) {
	if (md.normal_map.tex_handler > 0) {
		vec3 nm = textureGrad(sampler2D(md.normal_map.tex_handler), uv, duvdx, duvdy).xyz;
		mat3 tbn = mat3(t, b, n);
		n = tbn * normalize(vec3(nm));

		b = cross(t, n);
		t = cross(n, b);
	}
}

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
	vec3 h = normalize(v + l);
	
	material_layer_descriptor layer = mat_layer_descriptor[head_layer];
	vec3 base_color = material_layer_base_color(layer, uv, duvdx, duvdy);

	float roughness = layer.roughness;
	float metallic = layer.metallic;
	float F0 = material_convert_ior_to_F0(layer.ior, 1.f);
	float anisotropy_ratio = layer.anisotropy_ratio;
	float sheen = layer.sheen * 4.f;
	float sheen_power = mix(5.f, 2.f, layer.sheen_power);

	vec3 specular_tint = vec3(1);
	vec3 sheen_tint = vec3(1);

	vec3 c_spec = F0 * mix(specular_tint, base_color, metallic);

	vec3 S;
	if (anisotropy_ratio != 1.0f) {
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

	vec3 D = base_color * disney_diffuse_brdf(n, v, l, h,
											  roughness);

	vec3 c_sheen = fresnel_schlick(dot(l,h)) * sheen_tint * sheen;

	vec3 brdf = S + (1.f - metallic) * (D + c_sheen);
	return brdf * irradiance * dot(n, l);
}