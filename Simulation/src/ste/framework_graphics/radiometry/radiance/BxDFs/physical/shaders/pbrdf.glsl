
#include "common.glsl"

struct brdf_descriptor {
	uint64_t tex_handler;
	int min_theta_in, max_theta_in;
};

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
