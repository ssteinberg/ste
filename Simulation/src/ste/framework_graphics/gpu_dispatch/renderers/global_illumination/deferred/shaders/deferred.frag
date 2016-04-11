
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require
// #extension GL_NV_shader_atomic_fp16_vector : require

const int light_buffers_first = 2;

#include "material.glsl"
#include "light.glsl"
#include "shadow.glsl"
//#include "voxels.glsl"

in vec2 tex_coords;

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 2) uniform sampler2D color_tex;
layout(binding = 3) uniform sampler2D tangent_tex;
layout(binding = 4) uniform isampler2D mat_idx_tex;

layout(binding = 8) uniform sampler2DArray penumbra_layers;

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

		vec3 l = diffuse * ld.diffuse.xyz;

		float obscurance = texture(penumbra_layers, vec3(tex_coords, i)).x;

		float brdf = calc_brdf(md, position, n, t, b, v);
		float attenuation_factor = light_attenuation_factor(ld, dist);
		float incident_radiance = ld.luminance / attenuation_factor * obscurance;

		rgb += l * max(0.f, mix(.3f, 1.f, specular) * brdf * incident_radiance);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);


	// vec2 p = gl_FragCoord.xy / vec2(1688, 950);
	// p = p * 2 - vec2(1);
	// vec3 D = normalize((inv_view_model * inv_projection * vec4(p, 0, 1)).xyz);
	// vec3 P = vec3(0);
	// float radius = 0;

	// vec3 normal;
	// vec4 color;

	// bool hit;
	// float ray_length;

	// P = voxel_cone_march(P, D, vec3(0), radius, 0.06, radius, hit, ray_length);
	// //P = voxel_ray_march(P, D, vec3(0), hit, ray_length);

	// voxel_filter(P, radius, color, normal);

	// xyY = XYZtoxyY(RGBtoXYZ(vec3(ray_length / 100.f)));
	// xyY.z *= 1000;
	// gl_FragColor = color;
}
