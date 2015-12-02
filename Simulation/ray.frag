
#type frag
#version 450
#extension GL_ARB_bindless_texture : enable
#extension GL_NV_shader_atomic_fp16_vector : require
#extension GL_NV_gpu_shader5 : require

#include "voxels.glsl"

out vec4 gl_FragColor;

uniform mat4 inv_projection, inv_view_model;
uniform vec3 translation;

void main() {
	vec2 p = gl_FragCoord.xy / vec2(1688, 950);
	p = p * 2 - vec2(1);
	vec3 P = normalize((inv_view_model * inv_projection * vec4(p, 0, 1)).xyz);

    gl_FragColor = voxel_raymarch(vec3(translation), P);
}
