
#version 450
#type frag

layout(set=0,binding=0) uniform sampler2D sam;

layout(location = 0) out vec4 out_frag_color;

layout(location = 0) in vec2 uv;

void main() {
	out_frag_color = vec4(pow(texture(sam, uv).rgb, vec3(2.f)) * .2f, 1.f);
}
