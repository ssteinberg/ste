
#type frag
#version 450

out vec2 gl_FragColor;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 2) uniform sampler2D z_buffer;

uniform float aperature_radius = .15f;

void main() {
	ivec2 texSize = textureSize(z_buffer, 0);
	float focal = texelFetch(z_buffer, texSize / 2, 0).x;

	float s = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	
	float C = aperature_radius * abs(focal - s) / s;
	float coc = clamp(smoothstep(0, 1, C), 0, 1);
	
	gl_FragColor = vec2(coc, s);
}
