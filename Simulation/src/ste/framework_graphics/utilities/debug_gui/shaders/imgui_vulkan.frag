
#type frag
#version 450 core

layout(set=0, binding=0) uniform sampler2D fonts;

layout(location = 0) in struct {
    vec4 color;
    vec2 uv;
} vert_out;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vert_out.color * texture(fonts, vert_out.uv.st);
}
