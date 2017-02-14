
#type compute
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_shader_atomic_fp16_vector : require
#extension GL_NV_gpu_shader5 : require

#include <voxels.glsl>

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uniform int tiles;
uniform int level;

void main() {
	ivec3 id = ivec3(gl_GlobalInvocationID);
	int tex_size = textureSize(voxel_space_radiance, int(level)).x;
	ivec3 center = ivec3(tex_size >> 1);

	ivec3 coordinates = id * 2 + center - tiles;
	ivec3 upsample_coords = coordinates / 2;
	
	vec4 data000 = texelFetch(voxel_space_data, coordinates + ivec3(0, 0, 0), int(level));
	vec4 data100 = texelFetch(voxel_space_data, coordinates + ivec3(1, 0, 0), int(level));
	vec4 data010 = texelFetch(voxel_space_data, coordinates + ivec3(0, 1, 0), int(level));
	vec4 data110 = texelFetch(voxel_space_data, coordinates + ivec3(1, 1, 0), int(level));
	vec4 data001 = texelFetch(voxel_space_data, coordinates + ivec3(0, 0, 1), int(level));
	vec4 data101 = texelFetch(voxel_space_data, coordinates + ivec3(1, 0, 1), int(level));
	vec4 data011 = texelFetch(voxel_space_data, coordinates + ivec3(0, 1, 1), int(level));
	vec4 data111 = texelFetch(voxel_space_data, coordinates + ivec3(1, 1, 1), int(level));
	
	vec4 color000 = texelFetch(voxel_space_radiance, coordinates + ivec3(0, 0, 0), int(level));
	vec4 color100 = texelFetch(voxel_space_radiance, coordinates + ivec3(1, 0, 0), int(level));
	vec4 color010 = texelFetch(voxel_space_radiance, coordinates + ivec3(0, 1, 0), int(level));
	vec4 color110 = texelFetch(voxel_space_radiance, coordinates + ivec3(1, 1, 0), int(level));
	vec4 color001 = texelFetch(voxel_space_radiance, coordinates + ivec3(0, 0, 1), int(level));
	vec4 color101 = texelFetch(voxel_space_radiance, coordinates + ivec3(1, 0, 1), int(level));
	vec4 color011 = texelFetch(voxel_space_radiance, coordinates + ivec3(0, 1, 1), int(level));
	vec4 color111 = texelFetch(voxel_space_radiance, coordinates + ivec3(1, 1, 1), int(level));

	vec4 data = data000 +
				data100 +
				data010 +
				data110 +
				data001 +
				data101 +
				data011 +
				data111; 
	vec4 color =color000 +
				color100 +
				color010 +
				color110 +
				color001 +
				color101 +
				color011 +
				color111; 
	
	imageStore(voxel_data_levels(level + 1), upsample_coords, f16vec4(data));
	imageStore(voxel_radiance_levels(level + 1), upsample_coords, f16vec4(color));
}
