
#type frag
#version 450 core

layout(location = 0) out vec4 fColor;

layout(set=0, binding=0) uniform sampler2D fonts;

layout(location = 0) in struct {
    vec4 color;
    vec2 uv;
} In;

void main() {
    fColor = In.Color * texture(fonts, In.uv.uv);
}
