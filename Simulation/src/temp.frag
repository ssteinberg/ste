#version 450
#type frag

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 Tex;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform uniform_buffer_object {
    vec4 data;
} ubo;

void main() {
    outColor = vec4(.1f)+ vec4(fragColor * textureLod(texSampler, Tex, abs(ubo.data.x) * 5).rgb, 1.0);
}
