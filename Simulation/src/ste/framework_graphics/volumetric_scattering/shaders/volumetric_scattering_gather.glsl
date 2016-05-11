
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "volumetric_scattering.glsl"

layout(rgba16f, binding = 7) restrict uniform image3D volume;
layout(binding = 11) uniform sampler2DShadow depth_map;

void write_out(ivec3 p, vec4 rgba) {
	float scatter = exp(-rgba.a);
	imageStore(volume, p, vec4(rgba.rgb, scatter));
}

vec4 accumulate(vec4 front, vec4 back) {
	float scatter = exp(-front.a);
	vec3 l = front.rgb + clamp(scatter, .0f, 1.f) * back.rgb;
	return vec4(l, front.a + back.a);
}

void main() {
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);

	ivec3 p = ivec3(slice_coords, 0);
	vec4 rgba = imageLoad(volume, p);
	write_out(p, rgba);

	++p.z;
	for (; p.z < volumetric_scattering_depth_tiles; ++p.z) {
		vec4 next_rgba = imageLoad(volume, p);
		rgba = accumulate(rgba, next_rgba);
		// write_out(p, rgba);
		write_out(p, (p.z % 2 == 0) ? vec4(0,0,1,0) : vec4(1,0,0,0));
	}
}
