
uniform uint voxels_step_texels;
uniform float voxels_voxel_texel_size;
uniform uint voxels_texture_levels;

#include "material.glsl"
#include "utils.glsl"

layout(bindless_sampler) uniform sampler3D voxel_space_radiance;
layout(bindless_sampler) uniform usampler3D voxel_space_data;
uniform uint64_t voxel_radiance_levels_handles[8];
uniform uint64_t voxel_data_levels_handles[8];

layout(rgba16f) image3D voxel_radiance_levels(uint level) {
	return layout(rgba16f) image3D(voxel_radiance_levels_handles[level]);
}

layout(r32ui) uimage3D voxel_data_levels(uint level) {
	return layout(r32ui) uimage3D(voxel_data_levels_handles[level]);
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

void voxelize(vec3 P, vec4 color, vec3 normal) {
	uint level = voxel_level(P);
	float size = voxel_size(level);
	ivec3 vtc = ivec3(round(P / size)) + ivec3(imageSize(voxel_radiance_levels(level)).x >> 1);
	
	/*uint un = vec3ToUint(normal) << 8U;
	uint prev_val = 0, cur_val;
	uint new_val = un + 1;
	while ((cur_val = imageAtomicCompSwap(voxel_data_levels(level), vtc, prev_val, new_val)) != prev_val && cur_val != 0) {
		prev_val = cur_val;
		uint count = ((cur_val & 0xFF) + 1) & 0xFF;
		new_val = count + un;
	}*/
	
	//for (; level < voxels_texture_levels; ++level, vtc/=2) {
		imageAtomicAdd(voxel_radiance_levels(level), vtc, f16vec4(color));
		//imageStore(voxel_radiance_levels(level), vtc, f16vec4(color));
	//}
}

void voxelize(material_descriptor md, vec3 P, vec2 uv, vec3 normal, float coverage, vec2 dx, vec2 dy) {
	vec3 diffuse = md.diffuse.tex_handler>0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, dx, dy).rgb : vec3(1.f);

	voxelize(P, vec4(diffuse, 1) * coverage, normal);
}

vec3 voxel_raymarch(vec3 P, vec3 dir) {
	/*vec4 data = vec4(0);

	while (true) {
		vec3 p = P / size;
		ivec3 coordinates = ivec3(round(p)) + ivec3(textureSize(voxel_space, int(level)).x >> 1);

		data = texelFetch(voxel_space, coordinates, int(level));
		if (data.a > .0f) {
			if (level == 0)
				return data;
			--level;
			size /= 2.f;
		}
		else if (level < voxels_texture_levels) {
			++level;
			size *= 2.f;
			P += size * dir;
		}
	}*/

	while (true) {
		uint level = voxel_level(P);
		float size = voxel_size(level);
		int tex_size = textureSize(voxel_space_radiance, int(level)).x;
		ivec3 center = ivec3(tex_size >> 1);
		
		ivec3 coordinates = ivec3(round(P / size));

		ivec3 vtc = coordinates + center;
		vec4 data = textureLod(voxel_space_radiance, vec3(vtc) / vec3(tex_size), int(level));
		if (data.a > .0f) {
			//uint d = texelFetch(voxel_space_data, vtc, int(level)).x;
			//float count = float(d & 0xFF);
			return data.rgb / data.a;
		}

		P += dir * size;

		const float far = 4096.f;
		if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far) 
			return vec3(1,0,1);
	}
}
