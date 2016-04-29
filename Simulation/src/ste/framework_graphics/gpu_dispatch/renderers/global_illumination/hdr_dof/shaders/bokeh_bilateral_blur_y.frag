
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

#include "hdr_blur.glsl"

out mediump vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D hdr;
layout(bindless_sampler) uniform sampler2D zcoc_buffer;

void main() {
	// ivec2 lll_coords = ivec2(gl_FragCoord.xy) / 8;

	// int i;
	// uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	// for (i = 0; i < max_active_lights_per_frame; ++i, ++lll_ptr) {
	// 	lll_element lll_p = lll_buffer[lll_ptr];
	// 	if (lll_eof(lll_p))
	// 		break;
	// }

	// gl_FragColor = vec4(bokeh_blur(hdr, zcoc_buffer, ivec2(0,1)).xy, float(i) / 11.f, 1);

	gl_FragColor = bokeh_blur(hdr, zcoc_buffer, ivec2(0,1));
}
