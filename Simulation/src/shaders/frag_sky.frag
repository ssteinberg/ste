
#type frag
#version 450

in vec2 frag_texcoords;
in vec3 frag_position;
in float frag_depth;

layout(binding = 0) uniform sampler2D sky_tex;

layout(location = 0) out vec3 o_frag_position;
layout(location = 1) out vec4 o_frag_color;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;
layout(location = 4) out vec3 o_frag_tangent;
layout(location = 5) out int o_frag_mat_idx;

out vec4 gl_FragColor;

uniform float sky_luminance;

void main() {
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;
	
	o_frag_color = vec4(texture(sky_tex, uv).rgb * sky_luminance, 1);
	o_frag_mat_idx = -1;
	o_frag_z = frag_depth;
}
