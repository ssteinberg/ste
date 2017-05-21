#version 450
#type vert

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 Tex;

layout(binding = 0) uniform uniform_buffer_object {
    vec4 data;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	mat2 m = mat2(ubo.data.xy, ubo.data.zw);
    gl_Position = vec4(m * inPosition, 0.0, 1.0);
    fragColor = inColor;
	Tex = inTex;
}
