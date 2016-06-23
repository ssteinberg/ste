
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "chromaticity.glsl"

#include "material.glsl"

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

layout(std430, binding = 1) restrict readonly buffer material_layer_data {
	material_layer_descriptor mat_layer_descriptor[];
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

#include "material_evaluate.glsl"


layout(bindless_sampler) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(bindless_sampler) uniform samplerCubeArray shadow_maps;
layout(bindless_sampler) uniform sampler3D scattering_volume;

layout(binding = 0) uniform sampler2D back_face_depth;
layout(binding = 1) uniform sampler2D front_face_depth;

out vec4 gl_FragColor;

float get_thickness(ivec2 coord) {
	float fd = unproject_depth(texelFetch(front_face_depth, coord, 0).x);
	float bd = unproject_depth(texelFetch(back_face_depth, coord, 0).x);

	return fd - bd;
}

vec3 deferred_shade_fragment(g_buffer_element frag) {
	int draw_idx = gbuffer_parse_material(frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	float depth = gbuffer_parse_depth(frag);
	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(backbuffer_size()));
	vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, position);

	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, uv, duvdx, duvdy, n, t, b);

	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	vec3 rgb = material_emission(md);

	ivec2 lll_coords = ivec2(gl_FragCoord.xy) / lll_image_res_multiplier;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;

		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x) {
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			vec3 incident = light_incidant_ray(ld, position);
			if (dot(n, incident) <= 0)
				continue;

			float light_effective_range = ld.effective_range;
			float dist2 = dot(incident, incident);
			if (dist2 >= light_effective_range*light_effective_range)
				continue;

			float l_radius = ld.radius;
			vec3 shadow_v = w_pos - ld.position;
			float shdw = shadow(shadow_depth_maps,
								shadow_maps,
								uint(lll_parse_ll_idx(lll_p)),
								shadow_v,
								l_radius);
			if (shdw <= .0f)
				continue;

			float dist = sqrt(dist2);

			vec3 v = normalize(-position);
			vec3 l = incident / dist;
			vec3 irradiance = light_irradiance(ld, dist) * cavity * shdw;
			
			rgb += material_evaluate_radiance(head_layer,
											  n, t, b,
											  v, l,
											  uv, duvdx, duvdy,
											  irradiance);
		}
	}

	rgb = volumetric_scattering(scattering_volume, rgb, vec2(gl_FragCoord.xy), depth);

	return rgb;
}

void main() {
	g_buffer_element g_frag = gbuffer_load(ivec2(gl_FragCoord.xy));

	vec3 shaded_fragment = deferred_shade_fragment(g_frag);
	vec3 xyY = XYZtoxyY(RGBtoXYZ(shaded_fragment));

	gl_FragColor = vec4(xyY, 1);
}
