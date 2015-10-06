
#version 450

layout(location = 0) out vec3 o_frag_color;

in vec2 tex_coords;
layout(binding = 0) uniform sampler2D tex;

void main() {
	float l = texture(tex, tex_coords).z;
	o_frag_color = vec3(l,l,l);
}
