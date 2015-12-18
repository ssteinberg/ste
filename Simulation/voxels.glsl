
uniform uint voxels_step_texels;
uniform float voxels_voxel_texel_size;
uniform uint voxels_texture_levels;

#include "material.glsl"
#include "utils.glsl"

layout(bindless_sampler) uniform sampler3D voxel_space_radiance;
layout(bindless_sampler) uniform sampler3D voxel_space_data;
uniform uint64_t voxel_radiance_levels_handles[8];
uniform uint64_t voxel_data_levels_handles[8];

layout(rgba16f) image3D voxel_radiance_levels(uint level) {
	return layout(rgba16f) image3D(voxel_radiance_levels_handles[level]);
}

layout(rgba16f) image3D voxel_data_levels(uint level) {
	return layout(rgba16f) image3D(voxel_data_levels_handles[level]);
}

uint voxel_level(float d) {
	return min(uint(floor(d / voxels_voxel_texel_size / voxels_step_texels)), voxels_texture_levels);
}

uint voxel_level(vec3 P) {
	P = abs(P);
	float d = max(P.x, max(P.y, P.z));
	return voxel_level(d);
}

float voxel_size(uint level) {
	return (1 << level) * voxels_voxel_texel_size;
}

float voxel_size(vec3 P) {
	return voxel_size(voxel_level(P));
}

void voxelize(ivec3 C, vec4 color, vec3 normal, uint level, float size, float coverage) {
	ivec3 vtc = C + ivec3(imageSize(voxel_radiance_levels(level)).x >> 1);
	
	imageAtomicAdd(voxel_data_levels(level), vtc, f16vec4(normal * coverage, coverage));
	imageAtomicAdd(voxel_radiance_levels(level), vtc, f16vec4(color * coverage));
}

void voxelize(material_descriptor md, ivec3 C, vec2 uv, vec3 normal, uint level, float size, float coverage, vec2 dx, vec2 dy) {
	vec4 diffuse = md.diffuse.tex_handler > 0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, dx, dy) : vec4(1.f);
	voxelize(C, diffuse, normal, level, size, coverage);
}

void voxelize(material_descriptor md, vec3 P, vec2 uv, vec3 normal, float coverage, vec2 dx, vec2 dy) {
	uint level = voxel_level(P);
	float size = voxel_size(level);
	voxelize(md, ivec3(round(P / size)), uv, normal, level, size, coverage, dx, dy);
}

vec4 voxel_raymarch(vec3 P, vec3 dir) {
	while (true) {
		uint level = voxel_level(P);
		float size = voxel_size(level);
		int tex_size = textureSize(voxel_space_radiance, int(level)).x;
		ivec3 center = ivec3(tex_size >> 1);
		
		vec3 coordinates = P / size;
		vec3 rcoords = round(coordinates);

		vec4 data = texelFetch(voxel_space_data, ivec3(rcoords + center), int(level));
		if (data.w > .0f) {
			vec4 color = texelFetch(voxel_space_radiance, ivec3(rcoords + center), int(level)) / data.w;
			vec3 normal = data.xyz / data.w;

			return color;
		}

		P += dir * size / 6;
		
		const float far = 4096.f;
		if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far) 
			return vec4(0);
	}
}
