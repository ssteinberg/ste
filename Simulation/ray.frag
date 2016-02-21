
#type frag
#version 450
#extension GL_ARB_bindless_texture : enable
#extension GL_NV_shader_atomic_fp16_vector : require
#extension GL_NV_gpu_shader5 : require

#include "voxels.glsl"

out vec4 gl_FragColor;

uniform mat4 inv_projection, inv_view_model;

void main() { 
	vec2 p = gl_FragCoord.xy / vec2(1688, 950);
	p = p * 2 - vec2(1);
	vec3 D = normalize((inv_view_model * inv_projection * vec4(p, 0, 1)).xyz);
	vec3 P = vec3(0);
	float radius = 0;

	vec3 normal;
	vec4 color;

	bool hit;
	float ray_length;

	P = voxel_cone_march(P, D, vec3(0), radius, 0.06, radius, hit, ray_length);
	//P = voxel_ray_march(P, D, vec3(0), hit, ray_length);

	voxel_filter(P, radius, color, normal);

	gl_FragColor = color;
}
