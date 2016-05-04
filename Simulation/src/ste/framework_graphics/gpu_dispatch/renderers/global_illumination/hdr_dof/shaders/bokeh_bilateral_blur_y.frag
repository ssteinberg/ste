
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

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
