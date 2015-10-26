
#type frag
#version 450

#include "hdr_common.glsl"

out vec2 gl_FragColor;

in vec2 tex_coords;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 2) uniform sampler2D z_buffer;
layout(binding = 0) uniform atomic_uint counter;
layout(std430, binding = 0) buffer coc_descriptor {
	bokeh_point_descriptor points[];
};
layout(std430, binding = 2) coherent readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

uniform float bokeh_threshold = .2f;
uniform float coc_threshold = .09f;
uniform float aperature_radius = .05f;
uniform float f1 = .1f;

void main() {
	float focal = params.focus;

	float s = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	
	float C = aperature_radius * abs(focal - s) / s;
	float c = C * f1 / focal;
	float coc = clamp(smoothstep(0, 1, c), 0, 1);
	
	gl_FragColor = vec2(s, coc);

	/*vec4 hdr_texel = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);

	float lsum = 0;
	for (int x = -1; x<=1; ++x)
		for (int y = -1; y<=1; ++y) {
			if (x==0 && y==0) continue;
			lsum += texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(x,y), 0).w;
		}
	lsum /= 8;
	
	float lum_diff = hdr_texel.w - lsum;
	if (coc > coc_threshold && lum_diff > bokeh_threshold && hdr_texel.w > .8f) {
		uint idx = atomicCounterIncrement(counter);
		if (idx < 100000) {
			float x1 = (lum_diff - bokeh_threshold) / (bokeh_threshold * 4);
			float x2 = (coc - coc_threshold) / (coc_threshold * 2);
			float stepper = min(min(x1, x2), 1);
			float a = mix(.0f, .5f, stepper);

			points[idx].pos_size.xyz = vec3(tex_coords * 2 - vec2(1,1), coc);
			points[idx].color = vec4(hdr_texel.rgb, a);
		}
	}*/
}
