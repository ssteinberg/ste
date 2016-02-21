
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require
#extension GL_NV_shader_atomic_fp16_vector : require

const int light_buffers_first = 1;

#include "material.glsl"
#include "light.glsl"
#include "voxels.glsl"

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 2) uniform sampler2D color_tex;
layout(binding = 3) uniform sampler2D tangent_tex;
layout(binding = 4) uniform isampler2D mat_idx_tex;

uniform mat4 inv_projection, inv_view_model;

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
	
		float shadow_dist;
		bool hit;
		voxel_ray_march(position, vec3(0), light_transform_buffer[i].xyz, hit, shadow_dist, true);

		float brdf = calc_brdf(md, position, n, t, b, v);
		float attenuation_factor = light_attenuation_factor(ld, dist);
		float incident_radiance = ld.luminance / attenuation_factor;

		vec3 l = diffuse * ld.diffuse.xyz;
		if (hit)
			l *= pow(1.f / max(500 - shadow_dist, 1.f), 5.f);

		rgb += l * max(0, mix(.3f, 1.f, specular) * brdf * incident_radiance);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(rgb));
	//xyY.z = max(min_luminance, xyY.z);

	//gl_FragColor = vec4(xyY, 1);

	
	vec2 p = gl_FragCoord.xy / vec2(1688, 950);
	p = p * 2 - vec2(1);
	vec3 D = normalize((inv_view_model * inv_projection * vec4(p, 0, 1)).xyz);
	vec3 P = vec3(0);
	float radius = 0;

	vec3 normal;
	vec4 color;

	bool hit;
	float ray_length;

	P = voxel_cone_march(P, D, vec3(0), radius, 0.06, radius, hit, ray_length);
	//P = voxel_ray_march(P, D, vec3(0), hit, ray_length);

	voxel_filter(P, radius, color, normal);

	xyY = XYZtoxyY(RGBtoXYZ(vec3(ray_length / 100.f)));
	xyY.z *= 1000;
	gl_FragColor = color;
}
