
uniform uint voxels_step_texels;
uniform float voxels_voxel_texel_size;
uniform uint voxels_texture_levels;
uniform uint voxels_texture_size;
uniform vec3 voxels_world_translation;
uniform float voxels_world_size;

#include "material.glsl"

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
	vec4 diffuse = material_base_color(md, height_map_scale, uv, dx, dy);
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

void __voxel_ray_trace_step(inout vec3 P, inout float len, vec3 dir, vec3 inv_dir, float size) {
	float step_length;
	__voxel_trace_step(P, dir, inv_dir, size, step_length);
	len += step_length;
}

void __voxel_cone_trace_step(inout vec3 P, inout float len, inout float radius, float tan_theta, vec3 dir, vec3 inv_dir, float size) {
	float step_length;
	__voxel_trace_step(P, dir, inv_dir, size, step_length);
	radius += step_length * tan_theta;
	len += step_length;
}

void __voxel_filter(vec3 P, uint level, out vec4 color, out vec3 normal) {
	float size = voxel_size(level);

	vec3 P_over_size = P / size;
	vec3 frac = fract(P_over_size);

	ivec3 center = ivec3(textureSize(voxel_space_radiance, int(level)).x >> 1);
	ivec3 coordinates = center + ivec3(floor(P_over_size));

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

	vec4 dataX00 = mix(data000, data100, frac.x);
	vec4 dataX10 = mix(data010, data110, frac.x);
	vec4 dataX01 = mix(data001, data101, frac.x);
	vec4 dataX11 = mix(data011, data111, frac.x);
	vec4 dataXY0 = mix(dataX00, dataX10, frac.y);
	vec4 dataXY1 = mix(dataX01, dataX11, frac.y);

	vec4 colorX00 = mix(color000, color100, frac.x);
	vec4 colorX10 = mix(color010, color110, frac.x);
	vec4 colorX01 = mix(color001, color101, frac.x);
	vec4 colorX11 = mix(color011, color111, frac.x);
	vec4 colorXY0 = mix(colorX00, colorX10, frac.y);
	vec4 colorXY1 = mix(colorX01, colorX11, frac.y);

	vec4 data = mix(dataXY0, dataXY1, frac.z);

	color = mix(colorXY0, colorXY1, frac.z) / data.w;
	normal = data.xyz / data.w;
}

void voxel_filter(vec3 P, out vec4 color, out vec3 normal) {
	P += voxels_world_translation;

	uint level = voxel_level(P);
	__voxel_filter(P, level, color, normal);
}

void voxel_filter(vec3 P, float radius, out vec4 color, out vec3 normal) {
	P += voxels_world_translation;

	uint level = voxel_level(P);
	float cone_level = voxel_f_level_from_size(radius);
	uint icl = uint(floor(cone_level));
	level = max(level, icl);

	__voxel_filter(P, level, color, normal);

	if (level < voxels_texture_levels - 1 && level == icl) {
		float frac = fract(cone_level);
		if (frac > .0f) {
			vec4 color1;
			vec3 normal1;
			__voxel_filter(P, level + 1, color1, normal1);

			color = mix(color, color1, frac);
			normal = mix(normal, normal1, frac);
		}
	}
}

vec3 voxel_ray_march(vec3 P, vec3 dir, vec3 end_point, out bool hit, out float ray_length, bool trace_to_point = false, float max_length = .0f) {
	P += voxels_world_translation;
	end_point += voxels_world_translation;

	uint level = voxel_level(P);
	float size = voxel_size(level);
	ray_length = 0;
	hit = false;

	if (trace_to_point) {
		dir = end_point - P;
		float ldir = length(dir);
		if (ldir <= size)
			return P - voxels_world_translation;
		max_length = max_length > 0 ? min(ldir, max_length) : ldir;
		dir /= ldir;
	}

	vec3 inv_dir = 1.f / dir;

	__voxel_ray_trace_step(P, ray_length, dir, inv_dir, size);

	uint min_level = voxel_level(P);
	level = max(min_level, level);
	size = voxel_size(level);

	ivec3 center = ivec3(textureSize(voxel_space_radiance, int(level)).x >> 1);
	ivec3 rcoords = ivec3(floor(P / size));

	int counter = 1;

	while (true) {
		float texel_counter = texelFetch(voxel_space_data, rcoords + center, int(level)).w;
		if (texel_counter > .0f) {
			if (level == min_level) {
				hit = true;
				break;
			}

			--level;
			size *= .5f;
			center = center << 1;

			counter = 0;
			rcoords = ivec3(floor(P / size));

			continue;
		}

		__voxel_ray_trace_step(P, ray_length, dir, inv_dir, size);
		++counter;

		if (max_length > 0 && ray_length >= max_length)
			break;

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

		if (!trace_to_point) {
			const float far = voxels_world_size;
			if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far)
				break;
		}
	}

	return P - voxels_world_translation;
}

vec3 voxel_cone_march(vec3 P, vec3 dir, vec3 end_point, float start_radius, float tan_theta, out float radius, out bool hit, out float ray_length, bool trace_to_point = false, float max_length = .0f) {
	P += voxels_world_translation;
	end_point += voxels_world_translation;

	radius = start_radius;
	float cone_level = voxel_f_level_from_size(radius);
	uint icone_level = uint(floor(cone_level));
	uint min_level = voxel_level(P);
	uint level = max(icone_level, min_level);
	float size = voxel_size(level);

	ray_length = 0;
	hit = false;

	if (trace_to_point) {
		dir = end_point - P;
		float ldir = length(dir);
		if (ldir <= size)
			return P - voxels_world_translation;
		max_length = max_length > 0 ? min(ldir, max_length) : ldir;
		dir /= ldir;
	}

	vec3 inv_dir = 1.f / dir;

	__voxel_cone_trace_step(P, ray_length, radius, tan_theta, dir, inv_dir, size);

	cone_level = voxel_f_level_from_size(radius);
	icone_level = uint(floor(cone_level));
	min_level = max(voxel_level(P), icone_level);

	level = min_level;
	size = voxel_size(level);

	ivec3 center = ivec3(textureSize(voxel_space_radiance, int(level)).x >> 1);
	ivec3 rcoords = ivec3(floor(P / size));

	int counter = 1;

	while (true) {
		float texel_counter = texelFetch(voxel_space_data, rcoords + center, int(level)).w;
		if (texel_counter > .0f) {
			if (level == min_level) {
				hit = true;
				break;
			}

			--level;
			size *= .5f;
			center = center << 1;

			counter = 0;
			rcoords = ivec3(floor(P / size));

			continue;
		}

		__voxel_cone_trace_step(P, ray_length, radius, tan_theta, dir, inv_dir, size);
		++counter;

		if (max_length > 0 && ray_length >= max_length)
			break;

		if ((counter % 2) == 0 && level < voxels_texture_levels - 1) {
			++level;
			size *= 2.f;
			center = center >> 1;
		}

		cone_level = voxel_f_level_from_size(radius);
		icone_level = uint(floor(cone_level));
		min_level = max(voxel_level(P), icone_level);
		if (level < min_level) {
			++level;
			size = voxel_size(level);
			center = center >> 1;
		}

		rcoords = ivec3(floor(P / size));

		if (!trace_to_point) {
			const float far = voxels_world_size;
			if (abs(P.x) >= far || abs(P.y) >= far || abs(P.z) >= far)
				break;
		}
	}

	return P - voxels_world_translation;
}
