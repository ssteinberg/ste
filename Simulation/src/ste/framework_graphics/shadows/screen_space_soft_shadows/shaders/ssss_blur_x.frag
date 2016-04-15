
#type frag
#version 450

#include "ssss_blur.glsl"

out mediump vec4 gl_FragColor;

layout(binding = 7) uniform sampler2DArray src;

void main() {
	gl_FragColor = ssss_blur(src, gl_Layer, ivec2(1,0));
}
