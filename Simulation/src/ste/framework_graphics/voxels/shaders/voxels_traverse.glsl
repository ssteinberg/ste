
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

struct voxel_stack_t {
	vec3 t0;
	vec3 t1;
	uint node;
	uint brick;
	uint level;
};

uint first_node(vec3 t0, vec3 tm) {
	const bvec3 max_t0 = equal(max_element(t0).xxx, t0);
	const ivec3 a = ivec3(lessThan(tm.xxy, t0.zyx)) * ivec3(4, 4, 2);
	const ivec3 b = ivec3(lessThan(tm.yzz, t0.zyx)) * ivec3(2, 1, 1);
	const ivec3 nodes = a + b;

	return mix(mix(nodes.z, nodes.y, max_t0.y), 
			   nodes.x, max_t0.z);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse2(vec3 V, vec3 D, uint step_limit, float distance_limit, float step_length, float sin_theta, bool interpolate_data) {
	const bvec3 b_dir_lt_zero = lessThan(D, vec3(.0f));
	const vec3 dir = max(abs(D), vec3(1e-10f));
	const vec3 recp_dir = 1.f / dir;

	const uvec3 mask = uvec3(4,2,1);
	const uvec3 invert_dir = ivec3(b_dir_lt_zero) * mask;
	const uint a = invert_dir.x + invert_dir.y + invert_dir.z;
	
	vec3 v = 2.f * V / voxel_world;
	const vec3 o = v;
	v = mix(v, -v, b_dir_lt_zero);
	
	const vec3 it0 = (-1.f - v) * recp_dir;
	const vec3 it1 = ( 1.f - v) * recp_dir;
	const vec3 itm = mix(it0, it1, .5f);
	
	voxel_stack_t stack[voxel_leaf_level];
	stack[0].node = voxel_root_node;
	stack[0].level = 0;
	stack[0].t0 = it0;
	stack[0].t1 = it1;
	stack[0].brick = first_node(it0, itm);
	
	int j = 0;

	// Traverse
	uint i;
	for (i=0; i<step_limit; ++i) {
		vec3 t0 = stack[j].t0;
		vec3 t1 = stack[j].t1;
		const uint level = stack[j].level;
		const vec3 tm = mix(t0, t1, .5f);
		const uint b = stack[j].brick;

		const uvec3 t = b.xxx & mask;
		const bvec3 mixer = bvec3(t);
		t0 = mix(t0, tm, mixer);
		t1 = mix(tm, t1, mixer);
		
		const uvec3 step = t * uvec3(0,8,8) + b.xxx + mask;
		const uint current_child = b ^ a;

		const uint child_ptr = stack[j].node + current_child;
		const uint child = voxels_read(child_ptr);
		const bool push_down = all(bvec4(child != 0, greaterThanEqual(t1, vec3(.0f))));

		if (push_down && level == voxel_leaf_level - 1)
			break;

		const bvec3 min_t1 = equal(min_element(t1).xxx, t1);
		const uint next_brick = mix(mix(step.x, step.y, min_t1.y), 
									step.z, min_t1.z);

		if (next_brick >= 8) {
			--j;
		}
		else {
			stack[j].brick = next_brick;
		}

		if (push_down) {
			++j;
			stack[j].node = child;
			stack[j].level = level + 1;
			stack[j].brick = first_node(t0, mix(t0, t1, .5f));
			stack[j].t0 = t0;
			stack[j].t1 = t1;
		}
		else if (j < 0) {
			break;
		}
	}

	
	voxel_traversal_result_t ret;
	if (j >= 0) {
		vec3 t0 = stack[j].t0;
		vec3 t1 = stack[j].t1;
		const vec3 tm = mix(t0, t1, .5f);

		v = -tm * D;
		v = mix(v, -v, b_dir_lt_zero);

		const vec3 P = v * voxel_world * .5f;
		ret.distance = length(P - V);

		// Read and interpolate data for ray traces
		/*if (sin_theta == .0f)
			ret.data = voxel_interpolate_data(u, v, node, level, interpolate_data);

		// Write accumulated vacancy mask for cone traces
		if (sin_theta > .0f)
			ret.accumulated_vacancy = occupancy;*/
	}
	else {
		// No hit
		ret.distance = +inf;
	}
				
	return ret;
}


/**
* @brief Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse(vec3 V, vec3 D, uint step_limit, float step_length, float sin_theta, bool interpolate_data) {
	const bvec3 b_dir_lt_zero = lessThan(D, vec3(.0f));
	const vec3 dir = max(abs(D), vec3(1e-10f));
	const vec3 recp_dir = 1.f / dir;
 
	const uint n = voxel_leaf_level - 1;
	const float grid_res = float(voxel_resolution(n));
	
	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;
	v = mix(v, 1.f - v, b_dir_lt_zero);
	ivec3 u = ivec3(v * grid_res);
 
	// Init
	const uint P = voxel_P; 
	uint stack[voxel_leaf_level];
	stack[0] = 0;
 
	// Traverse
	int level = 0;
	for (uint i=0; i<step_limit; ++i) {
		const int level_resolution = int(P * (n - level));

		// Calculate brick coordinates
		const int mask = (1 << P) - 1;
		ivec3 b = (u >> level_resolution) & mask;
		b = mix(b, ivec3(mask) - b, b_dir_lt_zero);
		const uint brick_idx = voxel_brick_index(b, P);
 
		// Read child node address
		const uint node = stack[level];
		const uint child_ptr = node + voxel_node_children_offset(level, P) + brick_idx;
		const uint child = voxels_read(child_ptr);
 
		// Check if we have child
		if (child != 0) {
			// Step in
			++level;
			stack[level] = child;
   
			if (level == voxel_leaf_level) {
				// Hit leaf
				break;
			}
		}
		else {
			// No child, traverse.
			const float res = float(voxel_resolution(level));

			const vec3 f = fma(-v, res.xxx, vec3(u >> level_resolution));
			const vec3 t = fma(recp_dir, f, recp_dir);
			const float t_bar = step_length == .0f ? min_element(t) : step_length;
			const bvec3 mixer = equal(vec3(t_bar), t);
			const vec3 step = max(t_bar, 1e-3f) * dir;
 
			// Step v
			v += step / res;
 
			const int m = 1 << level_resolution;
			const ivec3 u_hat = (u | ivec3(m - 1)) + ivec3(1);
			const ivec3 u_bar = step_length == .0f ? mix(ivec3(v * grid_res), u_hat, mixer) : ivec3(v * grid_res);
  
			// Pop, if needed
			const int msb = max_element(findMSB(u_bar ^ u));
			const int pop = max(0, msb - level_resolution) / int(P);
			level -= pop;
			if (level < 0) {
				// No voxel was hit 
				break;
			}
  
			u = u_bar; 
		}
	}
 
 
	voxel_traversal_result_t ret;
	if (level > 0) {
		// Compute distance
		v = mix(v, 1.f - v, b_dir_lt_zero);
		const vec3 pos = (v - .5f) * voxel_world;
		ret.distance = length(pos - V);

		const uint node = stack[level];
 
		// Read and interpolate data for ray traces
		ret.data = voxel_interpolate_data(u, v, node, level, interpolate_data);
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
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  .0f, 
						  .0f, 
						  true);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any, without data interpolation.
*/
voxel_traversal_result_t voxel_traverse_ray_fast(vec3 V, vec3 dir, uint step_limit) {
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  .0f, 
						  .0f, 
						  false);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_ray_fixed_step(vec3 V, vec3 dir, float step_length, uint step_limit) {
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  step_length, 
						  .0f, 
						  true);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any, without data interpolation.
*/
voxel_traversal_result_t voxel_traverse_ray_fixed_step_fast(vec3 V, vec3 dir, float step_length, uint step_limit) {
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  step_length, 
						  .0f, 
						  false);
}
