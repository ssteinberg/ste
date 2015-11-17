
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

const int light_buffers_first = 1;

#include "material.glsl"
#include "light.glsl"

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 2) uniform sampler2D color_tex;
layout(binding = 3) uniform sampler2D tangent_tex;
layout(binding = 4) uniform isampler2D mat_idx_tex;

void main() {
	int draw_idx = texelFetch(mat_idx_tex, ivec2(gl_FragCoord.xy), 0).x;
	vec4 c = texelFetch(color_tex, ivec2(gl_FragCoord.xy), 0);

	vec3 diffuse = c.rgb;
	float specular = c.w;

	if (draw_idx < 0) {
		gl_FragColor = vec4(XYZtoxyY(RGBtoXYZ(diffuse)), 1);
		return;
	}

	vec3 n = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 t = texelFetch(tangent_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 b = cross(t, n);
	vec3 position = texelFetch(position_tex, ivec2(gl_FragCoord.xy), 0).xyz;

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 rgb = md.emission.rgb;
	for (int i = 0; i < light_buffer.length(); ++i) {
		light_descriptor ld = light_buffer[i];

		vec3 v = light_incidant_ray(ld, i, position);
		float dist = length(v);

		float brdf = calc_brdf(md, position, n, t, b, v);
		float attenuation_factor = light_attenuation_factor(ld, dist);
		float incident_radiance = ld.luminance / attenuation_factor;

		vec3 l = diffuse * ld.diffuse.xyz;
		rgb += l * max(0, mix(.3f, 1.f, specular) * brdf * incident_radiance);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);
}
