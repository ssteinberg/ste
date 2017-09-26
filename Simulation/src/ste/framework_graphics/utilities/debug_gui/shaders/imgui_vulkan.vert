
#type vert
#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;

layout(push_constant) uniform push_t {
    vec2 scale;
};

layout(location = 0) out struct {
    vec4 color;
    vec2 uv;
} vert_out;

void main() {
    vec2 translate = vec2(-1);

    vert_out.color = color;
    vert_out.uv = uv;
    gl_Position = vec4(pos * scale + translate, 0, 1);
}
