
#type frag
#version 450

#include "hdr_common.glsl"

out vec2 gl_FragColor;

in vec2 tex_coords;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 2) uniform sampler2D z_buffer;
layout(std430, binding = 2) coherent readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

uniform float aperature_radius = .05f;
uniform float f1 = .1f;

void main() {
	float focal = params.focus;

	float s = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	
	float C = aperature_radius * abs(focal - s) / s;
	float c = C * f1 / focal;
	float coc = clamp(smoothstep(0, 1, c), 0, 1);
	
	gl_FragColor = vec2(s, coc);
}
