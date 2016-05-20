
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"
#include "cook_torrance.glsl"
#include "oren_nayar.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

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

vec3 shade(g_buffer_element frag) {
	int draw_idx = gbuffer_parse_material(frag);
	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 diffuse = md.diffuse.tex_handler>0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, duvdx, duvdy).rgb : vec3(1.f);
	float depth = gbuffer_parse_depth(frag);

	float specular = md.specular.tex_handler>0 ? textureGrad(sampler2D(md.specular.tex_handler), uv, duvdx, duvdy).x : 1.f;
	specular = mix(.2f, 1.f, specular);

	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(backbuffer_size()));
	vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, position);

	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, height_map_scale, uv, duvdx, duvdy, n, t, b, position);

	float roughness = .5f;

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

			vec3 incident = light_incidant_ray(ld, position);
			if (dot(n, incident) <= 0)
				continue;

			float light_effective_range = ld.effective_range;
			float dist2 = dot(incident,incident);
			if (dist2 >= light_effective_range*light_effective_range)
				continue;

			vec3 shadow_v = w_pos - ld.position;
			float shadow = shadow(shadow_depth_maps,
								  uint(lll_parse_ll_idx(lll_p)),
								  shadow_v);
			if (shadow <= .0f)
				continue;

			float dist = sqrt(dist2);
			float l_radius = ld.radius;

			vec3 v = normalize(-position);
			vec3 l = incident / dist;

			float attenuation_factor = light_attenuation_factor(ld, dist);
			float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);
			vec3 irradiance = ld.diffuse * max(0.f, specular * incident_radiance * shadow);

			vec3 spec_brdf = cook_torrance_iso_brdf(n, v, l,
													roughness,
													diffuse);
			vec3 diff_brdf = oren_nayar_brdf(n, v, l,
											 roughness,
											 diffuse);
			vec3 brdf = spec_brdf + diff_brdf;

			rgb += brdf * irradiance * max(0.f, dot(n, l));
		}
	}

	rgb = volumetric_scattering(scattering_volume, rgb, vec2(gl_FragCoord.xy), depth);

	return rgb;
}

void main() {
	g_buffer_element frag = gbuffer_load(ivec2(gl_FragCoord.xy));
	vec3 c = shade(frag);

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);
}
