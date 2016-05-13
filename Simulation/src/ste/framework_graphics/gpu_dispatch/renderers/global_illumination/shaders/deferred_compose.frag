
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_matrix_buffer.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};
layout(shared, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};

layout(r32ui, binding = 7) restrict readonly uniform uimage2D gbuffer_ll_heads;
layout(shared, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};

layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(shared, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

#include "light_load.glsl"
#include "linked_light_lists_load.glsl"
#include "gbuffer_load.glsl"

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(binding = 9) uniform sampler3D scattering_volume;

out vec4 gl_FragColor;

uniform float height_map_scale = .5f;
uniform float proj00, proj11, proj23, shadow_proj23;

vec4 shade(g_buffer_element frag, mat4 inverse_view_matrix) {
	int draw_idx = gbuffer_parse_material(frag);
	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 diffuse = md.diffuse.tex_handler>0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, duvdx, duvdy).rgb : vec3(1.f);
	float alpha = gbuffer_parse_alpha(frag);
	float depth = gbuffer_parse_depth(frag);

	float specular = md.specular.tex_handler>0 ? textureGrad(sampler2D(md.specular.tex_handler), uv, duvdx, duvdy).x : 1.f;
	specular = mix(.2f, 1.f, specular);

	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(gbuffer_size(gbuffer_ll_heads)), proj23, proj00, proj11);
	vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;
	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);

	normal_map(md, height_map_scale, uv, duvdx, duvdy, n, t, b, position);

	vec3 rgb = md.emission.rgb;

	ivec2 lll_coords = ivec2(gl_FragCoord.xy) / lll_image_res_multiplier;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;

		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x &&
			depth <= lll_depth_range.y) {
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			vec3 v = light_incidant_ray(ld, light_idx, position);
			if (dot(n, v) > 0) {
				float dist = length(v);
				float l_radius = ld.radius;
				vec3 l = diffuse * ld.diffuse;

				vec3 shadow_v = w_pos - ld.position_direction.xyz;
				float shadow = shadow(shadow_depth_maps,
										uint(lll_parse_ll_idx(lll_p)),
										shadow_v,
										shadow_proj23);
				if (shadow <= .0f)
					continue;

				float brdf = calc_brdf(md, position, n, t, b, v / dist);
				float attenuation_factor = light_attenuation_factor(ld, dist);
				float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);

				float irradiance = specular * brdf * incident_radiance * shadow;
				rgb += l * max(0.f, irradiance);
			}
		}
	}

	vec4 vol_sam = volumetric_scattering_load_inscattering_transmittance(scattering_volume,
																		 vec2(gl_FragCoord.xy),
																		 depth);
	rgb = rgb * vol_sam.a + vol_sam.rgb;

	return vec4(rgb, alpha);
}

void main() {
	mat4 inverse_view_matrix = transpose(view_matrix_buffer.transpose_inverse_view_matrix);

	g_buffer_element frag = gbuffer_load(gbuffer_ll_heads, ivec2(gl_FragCoord.xy));
	vec4 c = shade(frag, inverse_view_matrix);

	uint32_t next_ptr = gbuffer_parse_nextptr(frag);
	while (c.a < 1.f && !gbuffer_eof(next_ptr)) {
		frag = gbuffer_load(next_ptr);
		vec4 c2 = shade(frag, inverse_view_matrix);

		c = mix(c2, c, c.a);

		next_ptr = gbuffer_parse_nextptr(frag);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c.rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);
}
