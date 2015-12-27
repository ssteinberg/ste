
uniform uint voxels_step_texels;
uniform float voxels_voxel_texel_size;
uniform uint voxels_texture_levels;
uniform uint voxels_texture_size;
uniform vec3 voxels_world_translation;
uniform float voxels_world_size;

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
	return min(uint(floor(d / voxels_voxel_texel_size / voxels_step_texels)), voxels_texture_levels - 1);
}

float voxel_f_level_from_size(float size) {
	return min(log2(size / voxels_voxel_texel_size), float(voxels_texture_levels - 1));
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

void voxelize(ivec3 C, vec4 color, vec3 normal, uint level, float coverage) {
	ivec3 vtc = C + ivec3(imageSize(voxel_radiance_levels(level)).x >> 1);
	
	imageAtomicAdd(voxel_data_levels(level), vtc, f16vec4(normal * coverage, coverage));
	imageAtomicAdd(voxel_radiance_levels(level), vtc, f16vec4(color * coverage));
}

void voxelize(material_descriptor md, ivec3 C, vec2 uv, vec3 normal, uint level, float coverage, vec2 dx, vec2 dy) {
	vec4 diffuse = md.diffuse.tex_handler > 0 ? textureGrad(sampler2D(md.diffuse.tex_handler), uv, dx, dy) : vec4(1.f);
	voxelize(C, diffuse, normal, level, coverage);
}

void voxelize(material_descriptor md, vec3 P, vec2 uv, vec3 normal, float coverage, vec2 dx, vec2 dy) {
	uint level = voxel_level(P);
	float size = voxel_size(level);
	voxelize(md, ivec3(floor(P / size)), uv, normal, level, coverage, dx, dy);
}

void __voxel_trace_step(inout vec3 P, vec3 dir, vec3 inv_dir, float size, out float step_length) {
	vec3 nP = P / size;
	vec3 p = (floor(nP) - nP) * inv_dir;
	p += (.5f * sign(dir) + vec3(.5f)) * inv_dir;

	bvec3 t = equal(vec3(0), p);
	p += vec3(t) * vec3(2.f);
	float d = min(p.x, min(p.y, p.z));

	step_length = d * size;
	P += (step_length + 0.001f) * dir;
}

void voxel_ray_trace_step(inout vec3 P, vec3 dir, vec3 inv_dir, float size) {
	float step_length;
	__voxel_trace_step(P, dir, inv_dir, size, step_length);
}

void voxel_cone_trace_step(inout vec3 P, inout float radius, float tan_theta, vec3 dir, vec3 inv_dir, float size) {
	float step_length;
	__voxel_trace_step(P, dir, inv_dir, size, step_length);
	radius += step_length * tan_theta;
}

vec4 voxel_ray_march(vec3 P, vec3 dir, int max_steps = 0) {
	P += voxels_world_translation;
	vec3 inv_dir = 1.f / dir;

	uint level = voxel_level(P);
	float size = voxel_size(level);
	
	voxel_ray_trace_step(P, dir, inv_dir, size);
	
	uint min_level = voxel_level(P);
	level = max(min_level, level);
	size = voxel_size(level);

	ivec3 center = ivec3(textureSize(voxel_space_radiance, int(level)).x >> 1);
	ivec3 rcoords = ivec3(floor(P / size));

	int counter = 1;
	int total_steps = 0;

	while (true) {
		vec4 data = texelFetch(voxel_space_data, rcoords + center, int(level));
		if (data.w > .0f) {
			if (level == min_level) {
				vec4 color = texelFetch(voxel_space_radiance, rcoords + center, int(level)) / data.w;
				vec3 normal = data.xyz / data.w;

				return color;
			}

			--level;
			size *= .5f;
			center = center << 1;

			counter = 0;
			rcoords = ivec3(floor(P / size));

			continue;
		}
		
		voxel_ray_trace_step(P, dir, inv_dir, size);
		++counter;
		++total_steps;
		
		if (max_steps > 0 && total_steps >= max_steps)
			return vec4(0);

		if ((counter % 2) == 0 && level < voxels_texture_levels - 1) {
			++level;
			size *= 2.f;
			center = center >> 1;
		}
		
		min_level = voxel_level(P);
		if (level < min_level) {
			++level;
			size = voxel_size(level);
			center = center >> 1;
		}

		rcoords = ivec3(floor(P / size));
		
		const float far = voxels_world_size;
		if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far) 
			return vec4(0);
	}
}

void __voxel_filter_cone_final_level(vec3 c, uint level, vec3 frac, out vec4 color, out vec3 normal) {
	c /= float(textureSize(voxel_space_data, int(level)));
	vec4 data = textureLod(voxel_space_data, c, int(level));
	color = textureLod(voxel_space_radiance, c, int(level)) / data.w;
	normal = data.xyz / data.w;
}

void __voxel_filter_cone_final(ivec3 center, uint level, float level_frac, vec3 P, float size, out vec4 color, out vec3 normal) {
	vec3 top_coords = P / size;
	vec3 frac_top = fract(top_coords);
	vec3 itop_coords = vec3(center) + top_coords;
	
	__voxel_filter_cone_final_level(itop_coords, level, frac_top, color, normal);

	uint min_level = voxel_level(P);
	if (level_frac == .0f)
		return;
	
	size *= 2.f;
	center = center >> 1;
	++level;
	vec3 bottom_coords = P / size;
	vec3 frac_bottom = fract(bottom_coords);
	vec3 ibottom_coords = vec3(center) + bottom_coords;
	vec4 bcolor;
	vec3 bnormal;
	
	__voxel_filter_cone_final_level(ibottom_coords, level, frac_bottom, bcolor, bnormal);
	
	color = mix(color, bcolor, level_frac);
	normal = mix(normal, bnormal, level_frac);
}

vec4 voxel_cone_march(vec3 P, vec3 dir, float start_radius, float tan_theta, int max_steps = 0) {
	P += voxels_world_translation;
	vec3 inv_dir = 1.f / dir;
	
	float r = start_radius;
	float cone_level = voxel_f_level_from_size(r);
	uint cone_level_floor = uint(floor(cone_level));
	uint min_level = voxel_level(P);
	uint level = max(cone_level_floor, min_level);
	float size = voxel_size(level);
	
	voxel_cone_trace_step(P, r, tan_theta, dir, inv_dir, size);
	cone_level = voxel_f_level_from_size(r);
	cone_level_floor = uint(floor(cone_level));
	min_level = max(voxel_level(P), cone_level_floor);

	level = min_level;
	size = voxel_size(level);

	ivec3 center = ivec3(textureSize(voxel_space_radiance, int(level)).x >> 1);
	ivec3 rcoords = ivec3(floor(P / size));

	int counter = 1;
	int total_steps = 0;

	while (true) {
		float texel_counter = texelFetch(voxel_space_data, rcoords + center, int(level)).w;
		if (texel_counter > .0f) {
			if (level == min_level) {
				vec4 color;
				vec3 normal;
				
				float level_frac = .0f;
				if (level < voxels_texture_levels - 1 && cone_level_floor == level)
					level_frac = fract(cone_level);
				__voxel_filter_cone_final(center, level, level_frac, P, size, color, normal);

				return color;
			}

			--level;
			size *= .5f;
			center = center << 1;

			counter = 0;
			rcoords = ivec3(floor(P / size));

			continue;
		}
		
		voxel_cone_trace_step(P, r, tan_theta, dir, inv_dir, size);
		++counter;
		++total_steps;

		if (max_steps > 0 && total_steps >= max_steps)
			return vec4(0);

		if ((counter % 2) == 0 && level < voxels_texture_levels - 1) {
			++level;
			size *= 2.f;
			center = center >> 1;
		}
		
		cone_level = voxel_f_level_from_size(r);
		cone_level_floor = uint(floor(cone_level));
		min_level = max(voxel_level(P), cone_level_floor);
		if (level < min_level) {
			++level;
			size = voxel_size(level);
			center = center >> 1;
		}

		rcoords = ivec3(floor(P / size));
		
		const float far = voxels_world_size;
		if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far) 
			return vec4(0);
	}
}
