
#include <common.glsl>
#include <voxels.glsl>


layout(set=1, binding = 0) uniform usampler2D voxels;


struct voxel_unpacked_data_t {
	vec3 normal;
	vec3 albedo;
	vec4 roughness_opacity_ior_metallicity;
};

struct voxel_traversal_result_t {
	float distance;
	uint level;
	float accumulated_vacancy;

	voxel_unpacked_data_t data;
};

uint voxels_read(uint ptr) {
	uint x = ptr & (voxel_buffer_line-1);
	uint y = ptr / voxel_buffer_line;
	return texelFetch(voxels, ivec2(x,y), 0).x;
}

voxel_unpacked_data_t mix(voxel_unpacked_data_t x, voxel_unpacked_data_t y, float a) {
	voxel_unpacked_data_t r;
	r.normal = mix(x.normal, y.normal, a);
	r.albedo = mix(x.albedo, y.albedo, a);
	r.roughness_opacity_ior_metallicity = mix(x.roughness_opacity_ior_metallicity, y.roughness_opacity_ior_metallicity, a);

	return r;
}

voxel_unpacked_data_t voxel_read_and_decode_data(uint node, uint level) {
	voxel_data_t data;

	uint data_ptr = node + voxel_node_data_offset(level, voxel_P);
	data.packed[0] = voxels_read(data_ptr + 0);
	data.packed[1] = voxels_read(data_ptr + 1);
	data.packed[2] = voxels_read(data_ptr + 2);
	data.packed[3] = voxels_read(data_ptr + 3);

	voxel_unpacked_data_t ret;
	decode_voxel_data(data, 
					  ret.albedo, 
					  ret.normal, 
					  ret.roughness_opacity_ior_metallicity.x, 
					  ret.roughness_opacity_ior_metallicity.y, 
					  ret.roughness_opacity_ior_metallicity.z, 
					  ret.roughness_opacity_ior_metallicity.w);

	return ret;
}

voxel_unpacked_data_t voxel_traverse_read_and_decode_data(ivec3 u, uint level, voxel_unpacked_data_t f) {
	uint node = voxel_root_node;
	int level_resolution = int(voxel_P * (level - 1));

	// Traverse
	for (uint l=0; l<level; ++l) {
		const float block = voxel_block_extent(l);
		const uint P = voxel_block_power(l);
		const ivec3 brick = (u >> level_resolution) % int(1 << P);

		const uint child_idx = voxel_brick_index(brick, P);
		const uint child_offset = node + voxel_node_children_offset(l, P) + child_idx;

		node = voxels_read(child_offset);
		if (node == 0) {
			return f;
		}
		
		level_resolution -= int(voxel_P);
	}
	
	// Read data
	return voxel_read_and_decode_data(node, level);
}

voxel_unpacked_data_t voxel_interpolate_data(ivec3 u, vec3 v, uint node, uint level, bool interpolate_data) {
	const float level_resolution = float(voxel_resolution(level - 1));
	const ivec2 step = ivec2(1, 0);

	const vec3 frac = v * level_resolution - (vec3(u) + .5f);
	const ivec3 s = ivec3(sign(frac));
	const vec3 f = abs(frac);

	// Read 8 corners
	voxel_unpacked_data_t d000 = voxel_read_and_decode_data(node, level);
	voxel_unpacked_data_t d001 = voxel_traverse_read_and_decode_data(u + step.xyy * s, level, d000);
	voxel_unpacked_data_t d010 = voxel_traverse_read_and_decode_data(u + step.yxy * s, level, d000);
	voxel_unpacked_data_t d100 = voxel_traverse_read_and_decode_data(u + step.yyx * s, level, d000);
	voxel_unpacked_data_t d011 = voxel_traverse_read_and_decode_data(u + step.xxy * s, level, mix(d010, d001, .5f));
	voxel_unpacked_data_t d101 = voxel_traverse_read_and_decode_data(u + step.xyx * s, level, mix(d001, d100, .5f));
	voxel_unpacked_data_t d110 = voxel_traverse_read_and_decode_data(u + step.yxx * s, level, mix(d010, d100, .5f));
	voxel_unpacked_data_t d111 = voxel_traverse_read_and_decode_data(u + step.yxx * s, level, mix(d011, d110, .5f));
	
	// Mix
	if (interpolate_data) {
		d000 = mix(d000, d001, f.x);
		d100 = mix(d100, d101, f.x);
		d010 = mix(d010, d011, f.x);
		d110 = mix(d110, d111, f.x);
	
		d000 = mix(d000, d010, f.y);
		d100 = mix(d100, d110, f.y);
	
		d000 = mix(d000, d100, f.z);
	}
	
	return d000;
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse(vec3 V, vec3 dir, uint step_limit, float step_length, float sin_theta, bool interpolate_data) {
	const bvec3 b_dir_gt_z = greaterThan(dir, vec3(.0f));
	const vec3 edge = mix(vec3(.0f), vec3(1.f), b_dir_gt_z);
	const vec3 recp_dir = 1.f / dir;
	const vec3 sign_dir = sign_ge_z(dir);

	const uint n = voxel_leaf_level - 1;
	const float grid_res = float(voxel_resolution(n));
	const float grid_res_0 = float(voxel_resolution(0));
	const vec2 res_initial = vec2(grid_res_0, 1.f / grid_res_0);
	const vec2 res_step = vec2(float(1 << voxel_P), 1.f / float(1 << voxel_P));

	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;
	const vec3 start_v = v;

	// Init
	uint node = voxel_root_node;
	uint P = voxel_Pi;
	int level_resolution = int(voxel_P * n);
	vec2 res = res_initial;

	ivec3 u = ivec3(v * grid_res);
	ivec3 b = (u >> level_resolution) % int(1 << voxel_Pi);

	float occupancy = 1.f;

	// Traverse
	int level = 0;
	for (uint i=0; i<step_limit; ++i) {
		// Is cone width greater than level resolution?
		const vec3 travelled = start_v - v;
		const float dist_squared = dot(travelled, travelled);
		const float w_squared = 4.f * sqr(sin_theta) * dist_squared;
		if (w_squared >= sqr(res.y) && level != 0 && sin_theta > .0f) {
			// Hit
			break;
		}

		// Calculate brick coordinates
		const uint brick_idx = voxel_brick_index(b, P);

		// Read child node address
		const uint child_ptr = node + voxel_node_children_offset(level, P) + brick_idx;
		const uint child = voxels_read(child_ptr);

		// Check if we have child here
		const bool has_child = child != 0;
		if (has_child) {
			// Step in
			++level;
			node = child;
			
			if (level == voxel_leaf_level) {
				// Hit leaf
				break;
			}

			// Read new level parameters
			level_resolution -= int(voxel_P);
			res *= res_step;
			P = voxel_P;

			b = (u >> level_resolution) % int(1 << voxel_P);

			continue;
		}

		// No child, traverse.
		const vec3 f = fma(-v, res.xxx, vec3(u >> level_resolution));
		const vec3 t = abs(recp_dir * max(fma(f, sign_dir, edge), vec3(1e-30f))); 		// Avoid nan created by division of 0 by inf
		const float t_bar = step_length == .0f ? min_element(t) : step_length;
		const bvec3 mixer = equal(vec3(t_bar), t);
		const vec3 step = max(t_bar, 1e-3f) * dir;

		// For cone traces: Apply occupancy mask
		if (sin_theta > .0f && w_squared >= sqr(res.y * res_step.y)) {
			const uint occupancy_offset = node + voxel_node_occupancy_offset(level, P);
			const uint occupancy_counter = voxels_read(occupancy_offset);
			const float occ = t_bar * float(occupancy_counter) / voxel_full_occupancy_factor(level);
			occupancy *= 1.f - min(occ, 1.f);
		}

		// Step v
		v += step * res.y;

		const int m = 1 << level_resolution;
		const ivec3 u_hat = (u | ivec3(m - 1)) + mix(-ivec3(m), ivec3(1), b_dir_gt_z);
		const ivec3 u_bar = step_length == .0f ? mix(ivec3(v * grid_res), u_hat, mixer) : ivec3(v * grid_res);
		
		ivec3 b_offset = (u_bar >> level_resolution) - (u >> level_resolution); 
        ivec3 b_bar = b + b_offset; 

		// Have we moved past the block edge?
		const bool past_edge = (b_bar & (~((1 << P) - 1))) != ivec3(0);
		if (past_edge) {
			if (level == 0) {
				// No voxel was hit 
				level = -1;
				break;
			}

			// Restart
			level = 0;
			node = voxel_root_node;
			P = voxel_block_power(level); 
			level_resolution = int(voxel_P * n);
			res = res_initial; 

			const ivec3 u_shift = u >> level_resolution; 
			b_offset = (u_bar >> level_resolution) - u_shift; 
			b_bar = u_shift % int(1 << P) + b_offset;
		}
		
		b = b_bar;
		u = u_bar; 
	}

	
	voxel_traversal_result_t ret;
	if (level > 0) {
		// Compute distance
		const vec3 pos = (v - .5f) * voxel_world;
		ret.distance = length(pos - V);
		ret.level = level;

		// Read and interpolate data for ray traces
		if (sin_theta == .0f)
			ret.data = voxel_interpolate_data(u, v, node, level, interpolate_data);

		// Write accumulated vacancy mask for cone traces
		if (sin_theta > .0f)
			ret.accumulated_vacancy = level < voxel_leaf_level ? occupancy : .0f;
	}
	else {
		// No hit
		ret.distance = +inf;
	}
				
	return ret;
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_ray(vec3 V, vec3 dir, uint step_limit) {
	return voxel_traverse(V, dir, step_limit, .0f, .0f, true);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_fixed_step(vec3 V, vec3 dir, float step_length, uint step_limit) {
	return voxel_traverse(V, dir, step_limit, step_length, .0f, true);
}

/**
*	@brief	Traverses a cone from V in direction dir with some apex angle.
*/
voxel_traversal_result_t voxel_traverse_cone(vec3 V, vec3 dir, float sin_theta, uint step_limit) {
	return voxel_traverse(V, dir, step_limit, .0f, sin_theta, false);
}

/**
*	@brief	Traverses a cone from V in direction dir with some apex angle.
*/
voxel_traversal_result_t voxel_traverse_cone_fixed_step(vec3 V, vec3 dir, float sin_theta, float step_length, uint step_limit) {
	return voxel_traverse(V, dir, step_limit, step_length, sin_theta, false);
}
