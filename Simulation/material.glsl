
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
};

float calc_brdf(material_descriptor md, vec3 position, vec3 normal, vec3 tangent, vec3 bitangent, vec3 incident) {
	float theta_min = md.brdf.min_theta_in;
	float theta_max = md.brdf.max_theta_in;
	
	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 v = incident;
	vec3 e = -position;
	
	vec3 win = normalize(v);
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

	if (out_theta < .1f)
		out_phi = mix(.0f, out_phi, clamp(out_theta / .05f, 0, 1));
	if (in_theta < .1f)
		out_phi = mix(.0f, out_phi, clamp(in_theta / .05f, 0, 1));

	sampler2DArray brdf_sampler = sampler2DArray(md.brdf.tex_handler);
	uint layers = textureSize(brdf_sampler, 0).z;

	float lf = in_theta * layers;
	float layer = floor(lf);
	vec3 tp = vec3(out_phi, out_theta, layer);
	
	vec3 offset = layer < layers - 1 ? vec3(0, 0, 1.f) : vec3(0);
	float l0 = texture(brdf_sampler, tp).x;
	float l1 = texture(brdf_sampler, tp + offset).x;

	return mix(l0, l1, fract(lf)) * cos_in_theta;
}
