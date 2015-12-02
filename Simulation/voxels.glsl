
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

void voxelize(vec3 P, vec4 color, vec3 normal, float coverage) {
	uint level = voxel_level(P);
	float size = voxel_size(level);
	ivec3 vtc = ivec3(round(P / size)) + ivec3(imageSize(voxel_radiance_levels(level)).x >> 1);
	
	imageAtomicAdd(voxel_radiance_levels(level), vtc, f16vec4(color * coverage));
	imageAtomicAdd(voxel_data_levels(level), vtc, f16vec4(normal * coverage, coverage));

	/*uint un = vec3ToUint(normal) << 8U;
	uint prev_val = 0, cur_val;
	uint new_val = un + 1;
	while ((cur_val = imageAtomicCompSwap(voxel_data_levels(level), vtc, prev_val, new_val)) != prev_val && cur_val != 0) {
		prev_val = cur_val;
		uint count = ((cur_val & 0xFF) + 1) & 0xFF;
		new_val = count + un;
	}*/
}

void voxelize(material_descriptor md, vec3 P, vec2 uv, vec3 normal, float coverage, vec2 dx, vec2 dy) {
	vec3 diffuse = md.diffuse.tex_handler>0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, dx, dy).rgb : vec3(1.f);
	voxelize(P, vec4(diffuse, 1), normal, coverage);
}

vec4 voxel_raymarch(vec3 P, vec3 dir) {
	while (true) {
		uint level = voxel_level(P);
		float size = voxel_size(level);
		int tex_size = textureSize(voxel_space_radiance, int(level)).x;
		ivec3 center = ivec3(tex_size >> 1);
		
		ivec3 coordinates = ivec3(round(P / size));

		ivec3 vtc = coordinates + center;
		vec4 color = textureLod(voxel_space_radiance, vec3(vtc) / vec3(tex_size), int(level));
		if (color.a > .0f) {
			//uint d = texelFetch(voxel_space_data, vtc, int(level)).x;
			//float count = float(d & 0xFF);

			vec4 data = textureLod(voxel_space_data, vec3(vtc) / vec3(tex_size), int(level));
			vec3 normal = data.xyz / data.w;

			return color / data.w;
		}

		P += dir * size;

		const float far = 4096.f;
		if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far) 
			return vec4(0);
	}
}
