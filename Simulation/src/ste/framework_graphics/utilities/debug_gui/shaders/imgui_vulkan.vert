
#type vert
#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in uint color;

layout(push_constant) uniform push_t {
    vec2 scale;
    vec2 translate;
};

out gl_PerVertex{
    vec4 gl_Position;
};

layout(location = 0) out struct {
    vec4 color;
    vec2 uv;
} vert_out;

void main() {
    vert_out.color = vec4(1,0,1,1);
    vert_out.uv = uv;
    gl_Position = vec4(pos * scale + translate, 0, 1);
}
