
#include "hdr_common.glsl"

struct material_texture_descriptor {
	uint64_t tex_handler;
};
struct brdf_descriptor {
	uint64_t tex_handler;
	int min_theta_in, max_theta_in;
};
struct material_descriptor {
	material_texture_descriptor diffuse;
	material_texture_descriptor specular;
	material_texture_descriptor normalmap;
	material_texture_descriptor alphamap;
	brdf_descriptor brdf;
	vec4 emission;
};

const int material_none = 0xFFFFFFFF;

void normal_map(material_descriptor md, float height_map_scale, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b, inout vec3 P) {
	if (md.normalmap.tex_handler > 0) {
		vec4 normal_height = textureGrad(sampler2D(md.normalmap.tex_handler), uv, duvdx, duvdy);
		mat3 tbn = mat3(t, b, n);

		float h = normal_height.w * height_map_scale;
		P += h * n;

		vec3 nm = normal_height.xyz;
		n = tbn * nm;

		t = cross(n, b);
		b = cross(t, n);
	}
}

vec3 oren_nayar_brdf(vec3 n,
				 	 vec3 v,
				 	 vec3 l,
				 	 float roughness,
				 	 vec3 albedo) {
	float roughness2 = roughness * roughness;
	float dotNL = dot(n,l);
	float dotVL = dot(v,l);
	float dotNV = dot(n,v);

	float s = max(0.f, dotVL - dotNV * dotNL);
	float t = min(1.f, dotNL / dotNV);

	float r1 = roughness2 / (roughness2 + 0.33f);
	float a = dotNL * (1.f - r1 * .5f);

	float b = (0.45f * roughness2 / (roughness2 + 0.09f)) * s * t;

	float d = a + b;

	return albedo * d / pi;
}

vec3 cook_torrance_brdf(vec3 n,
						vec3 v,
						vec3 l,
						float roughness,
						vec3 c_spec) {
	vec3 h = normalize(v + l);

	float dotNH = dot(n,h);
	float dotNV = dot(n,v);
	float dotNL = dot(n,l);
	float dotLH = dot(l,h);

	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float dotNH2 = dotNH * dotNH;
	float denom_ndf = dotNH2 * (alpha2 - 1.f) + 1.f;
	float d = alpha2 / (pi * denom_ndf * denom_ndf);

	float k = roughness * roughness / 4.f;
	float invk = 1.f - k;
	float g1 = 1.f / (dotNL * invk + k);
	float g2 = 1.f / (dotNV * invk + k);
	float g = g1 * g2;

	float p = 1.f - dotLH;
	float p2 = p*p;
	float p4 = p2*p2;
	float p5 = p4*p;
	vec3 f = c_spec + (vec3(1) - c_spec) * p5;

	return d * g * f / 4.f;
}

float pbrdf(sampler3D brdf,
			float theta_min,
			float theta_max,
			vec3 v,
			vec3 l,
			vec3 normal,
			vec3 tangent,
			vec3 bitangent) {
	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 win = l;
	vec3 wout = v;
	vec3 lwin = TBN * win;
	vec3 lwout = TBN * wout;

	float out_phi = .0f;
	float tl = length(lwin.xy);
	vec2 t = lwin.xy / tl;
	mat2 rotation_mat = mat2(t.x, -t.y, t.y, t.x);
	vec2 s = normalize(rotation_mat * lwout.xy);
	out_phi = acos(s.x);
	out_phi /= 2*pi;
	out_phi += .5f;

	float cos_in_theta = dot(win, normal);
	float in_theta = acos(cos_in_theta) / pi_2;
	in_theta = clamp((in_theta - theta_min) / (theta_max - theta_min), .0f, 1.f);

	float cos_out_theta = max(.0f, dot(wout, normal));
	float out_theta = acos(cos_out_theta) / pi_2;

	vec3 tp = vec3(out_phi, out_theta, in_theta);
	float radiance = texture(brdf, tp).x;

	return radiance * max(0, cos_in_theta);
}
