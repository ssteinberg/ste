
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require
// #extension GL_NV_shader_atomic_fp16_vector : require

#include "shadow.glsl"
#include "material.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"
#include "gbuffer.glsl"
#include "girenderer_matrix_buffer.glsl"
//#include "voxels.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(std430, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(std430, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(r32ui, binding = 7) restrict readonly uniform uimage2D gbuffer_ll_heads;

layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(std430, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"
#include "gbuffer_load.glsl"

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;

in vs_out {
	vec2 tex_coords;
} vin;

out vec4 gl_FragColor;

uniform float scattering_ro = 0.0003f;
uniform float proj22, proj23;

vec4 shade(g_buffer_element frag, mat4 inverse_view_matrix) {
	uint16_t draw_idx = frag.material;
	vec4 c = frag.albedo;

	vec3 diffuse = c.rgb;
	float alpha = c.a;
	float specular = mix(.2f, 1.f, frag.specular);

	if (draw_idx == material_none)
		return vec4(diffuse, alpha);

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 n = frag.N;
	vec3 t = frag.T;
	vec3 b = cross(t, n);
	vec3 position = frag.P.xyz;
	vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;
	vec3 rgb = md.emission.rgb;

	ivec2 lll_coords = ivec2(gl_FragCoord.xy) / 8;

	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (int i = 0; i < max_active_lights_per_frame; ++i, ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			return vec4(rgb, alpha);

		if (position.z >= lll_p.z_min && position.z <= lll_p.z_max) {
			int light_idx = int(lll_p.light_idx);
			light_descriptor ld = light_buffer[light_idx];

			vec3 v = light_incidant_ray(ld, light_idx, position);
			if (dot(n, v) > 0) {
				float dist = length(v);
				float l_radius = ld.radius;
				vec3 l = diffuse * ld.diffuse.xyz;

				vec3 shadow_v = w_pos - ld.position_direction.xyz;
				float shadow = shadow_penumbra_width(shadow_depth_maps,
													int(lll_p.ll_idx),
													shadow_v,
													l_radius,
													dist,
													proj22,
													proj23);
				if (shadow == 1.f)
					continue;

				float brdf = calc_brdf(md, position, n, t, b, v / dist);
				float attenuation_factor = light_attenuation_factor(ld, dist);
				float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);

				float irradiance = specular * brdf * incident_radiance * (1.f - shadow);
				rgb += l * max(0.f, irradiance);
			}
		}
	}

	return vec4(rgb, alpha);
}

void main() {
	mat4 inverse_view_matrix = transpose(view_matrix_buffer.transpose_inverse_view_matrix);

	g_buffer_element frag = gbuffer_load(gbuffer_ll_heads, ivec2(gl_FragCoord.xy));
	vec4 c = shade(frag, inverse_view_matrix);

	while (c.a < 1.f && !gbuffer_eof(frag.next_ptr)) {
		frag = gbuffer_load(frag.next_ptr);
		vec4 c2 = shade(frag, inverse_view_matrix);

		c = mix(c2, c, c.a);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c.rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);
}
