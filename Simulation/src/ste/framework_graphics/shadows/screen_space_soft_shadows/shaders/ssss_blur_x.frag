
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "ssss_blur.glsl"

out vec2 gl_FragColor;

layout(binding = 7) uniform sampler2D z_buffer;
layout(binding = 8) uniform sampler2DArray src;

void main() {
	gl_FragColor = ssss_blur_decompose(src, z_buffer, gl_Layer, ivec2(1,0));
}
