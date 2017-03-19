#version 450
#type frag

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 Tex;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor * texture(texSampler, Tex).rgb, 1.0);
}
