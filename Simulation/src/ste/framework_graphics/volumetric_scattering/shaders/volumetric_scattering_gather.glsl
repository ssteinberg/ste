
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "volumetric_scattering.glsl"

layout(rgba16f, binding = 7) restrict uniform image3D volume;
layout(binding = 11) uniform sampler2D depth_map;

float depth3x3(ivec2 uv, int lod) {
	float d00 = texelFetchOffset(depth_map, uv, lod, ivec2(-1,-1)).x;
	float d10 = texelFetchOffset(depth_map, uv, lod, ivec2( 0,-1)).x;
	float d20 = texelFetchOffset(depth_map, uv, lod, ivec2( 1,-1)).x;
	float d01 = texelFetchOffset(depth_map, uv, lod, ivec2(-1, 0)).x;
	float d11 = texelFetch(depth_map, uv, lod).x;
	float d21 = texelFetchOffset(depth_map, uv, lod, ivec2( 1, 0)).x;
	float d02 = texelFetchOffset(depth_map, uv, lod, ivec2(-1, 1)).x;
	float d12 = texelFetchOffset(depth_map, uv, lod, ivec2( 0, 1)).x;
	float d22 = texelFetchOffset(depth_map, uv, lod, ivec2( 1, 1)).x;

	float a = min(min(d00, d10), min(d20, d01));
	float b = min(min(d11, d21), min(d02, d12));
	float c = min(a, b);
	return min(c, d22);
}

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

	int depth_lod = 2;
	float depth_buffer_d = depth3x3(slice_coords, depth_lod);
	int max_tile = min(int(ceil(volumetric_scattering_tile_for_depth(depth_buffer_d))) + 2, volumetric_scattering_depth_tiles);

	ivec3 p = ivec3(slice_coords, 0);
	vec4 rgba = imageLoad(volume, p);
	write_out(p, rgba);

	++p.z;
	for (; p.z < max_tile; ++p.z) {
		vec4 next_rgba = imageLoad(volume, p);
		rgba = accumulate(rgba, next_rgba);
		write_out(p, rgba);
	}
}
