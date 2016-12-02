
#type compute
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "volumetric_scattering.glsl"

layout(rgba16f, binding = 7) restrict uniform image3D volume;
layout(bindless_sampler) uniform sampler2D depth_map;

float depth3x3(vec2 uv, int lod) {
	float d00 = textureLodOffset(depth_map, uv, lod, ivec2(-1,-1)).x;
	float d10 = textureLodOffset(depth_map, uv, lod, ivec2( 0,-1)).x;
	float d20 = textureLodOffset(depth_map, uv, lod, ivec2( 1,-1)).x;
	float d01 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 0)).x;
	float d11 = textureLod(depth_map, uv, lod).x;
	float d21 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 0)).x;
	float d02 = textureLodOffset(depth_map, uv, lod, ivec2(-1, 1)).x;
	float d12 = textureLodOffset(depth_map, uv, lod, ivec2( 0, 1)).x;
	float d22 = textureLodOffset(depth_map, uv, lod, ivec2( 1, 1)).x;

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
	ivec3 volume_size = imageSize(volume);
	ivec2 slice_coords = ivec2(gl_GlobalInvocationID.xy);
	if (slice_coords.x >= volume_size.x ||
		slice_coords.y >= volume_size.y)
		return;

	int depth_lod = 2;
	float depth_buffer_d = depth3x3((vec2(slice_coords) + vec2(.5f)) / vec2(volume_size.xy), depth_lod);
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
