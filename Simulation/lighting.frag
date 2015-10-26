
#type frag
#version 440

#include "hdr_common.glsl"

in vec3 l_pos;

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 3) uniform sampler2D color_tex;
layout(binding = 4) uniform sampler2D tangent_tex;
layout(binding = 5) uniform sampler2D specular_tex;
layout(binding = 8) uniform sampler3D brdf_tex;

const float pi_2 = pi * .5f;
const float l_intensity = 1000;
const float light_radius = 1;

uniform float theta_min = 10.f * pi / 180.f;
uniform float theta_max = 70.f * pi / 180.f;

void main() {
	vec3 normal = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 tangent = texelFetch(tangent_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 position = texelFetch(position_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 color = texelFetch(color_tex, ivec2(gl_FragCoord.xy), 0).rgb;
	float specular = texelFetch(specular_tex, ivec2(gl_FragCoord.xy), 0).r;
	
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 v = l_pos - position;
	vec3 e = -position;
	
	vec3 win = normalize(v);
	vec3 wout = normalize(e);
	vec3 lwin = TBN * win;
	vec3 lwout = TBN * wout;

	float out_phi = .0f;
	float tl = length(lwin.xy);
	vec2 t = lwin.xy / tl;
	mat2 rotation_mat = mat2(t.x, t.y, -t.y, t.x);
	vec2 s = normalize(rotation_mat * lwout.xy);
	out_phi = acos(s.x);
	out_phi /= 2*pi;
	out_phi += .5f;

	float cos_in_theta = dot(win, normal);
	float in_theta = acos(cos_in_theta) / pi_2;
	in_theta = clamp((in_theta - theta_min) / (theta_max - theta_min), .0f, 1.f);
	
	float cos_out_theta = max(.0f, dot(wout, normal));
	float out_theta = acos(cos_out_theta) / pi_2;

	float brdf = texture(brdf_tex, vec3(out_phi, out_theta, in_theta)).x;
	
	float st_in = abs(2*pi * (1.0f - cos(atan(light_radius / length(v)) * .5f)));

	color.z *= max(0, smoothstep(0, .5f, specular) * brdf * st_in * l_intensity) + .00002f;

	gl_FragColor = vec4(color, 1);
}
