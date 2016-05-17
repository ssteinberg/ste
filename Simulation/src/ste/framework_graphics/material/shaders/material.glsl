
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

float calc_brdf(material_descriptor md, vec3 position, vec3 normal, vec3 tangent, vec3 bitangent, vec3 incident) {
	if (md.brdf.tex_handler == 0)
		return .0f;

	float theta_min = md.brdf.min_theta_in;
	float theta_max = md.brdf.max_theta_in;

	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 v = incident;
	vec3 e = -position;

	vec3 win = v;
	vec3 wout = normalize(e);
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
	float l = texture(sampler3D(md.brdf.tex_handler), tp).x;

	return l * max(0, cos_in_theta);
}
