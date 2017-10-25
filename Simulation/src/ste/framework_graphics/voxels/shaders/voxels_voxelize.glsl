
#include <voxels.glsl>


layout(r32ui, set=1, binding = 0) restrict uniform uimage2D voxels;
layout(std430, set=1, binding=1) restrict buffer voxel_counter_binding {
	uint voxel_buffer_size;
};


ivec2 voxels_image_coords(uint ptr) {
	uint x = ptr % voxel_buffer_line;
	uint y = ptr / voxel_buffer_line;
	return ivec2(x,y);
}


void voxel_voxelize_blend_user_data(uint node,
									uint level,
									voxel_data_t data) {
	uint data_ptr = node + voxel_node_data_offset(level, voxel_P);

	/*if (level == voxel_leaf_level) {
		// For leaf nodes there 
		voxel_buffer[data_ptr + 0] = data.normal_roughness_packed;
		voxel_buffer[data_ptr + 1] = data.rgba;
		return;
	}*/

	// Unpack input user data
	vec3 N;
	float roughness;
	vec4 rgba;
	decode_voxel_data(data, N, roughness, rgba);

	// Atomically blend with other data
	uint old_normal_roughness_packed = 0;
	uint new_normal_roughness_packed = data.normal_roughness_packed;
	while ((new_normal_roughness_packed = imageAtomicCompSwap(voxels, 
															  voxels_image_coords(data_ptr), 
															  old_normal_roughness_packed,
															  new_normal_roughness_packed)) != old_normal_roughness_packed) {
		vec3 dest_N;
		float dest_roughness;
		decode_voxel_data_normal_roughness(new_normal_roughness_packed, dest_N, dest_roughness);


	}
}

/**
*	@brief	Traverses a node, atomically creating a child at supplied position.
*/
void voxel_voxelize(inout vec3 v,
					inout uint node, 
					uint level,
					voxel_data_t data) {
	const uint child_semaphore_lock = 0xFFFFFFFF;

	const float block = voxel_block_extent(level);
	const uint P = voxel_block_power(level);
		
	// Calculate brick coordinates and index in block
	const vec3 brick = v * block;
	const uint child_idx = voxel_brick_index(ivec3(brick), P);
	const uint child_offset = node + voxel_node_children_offset(P) + child_idx;

	const uint old_child = imageAtomicCompSwap(voxels, 
											   voxels_image_coords(child_offset),
											   0,
											   child_semaphore_lock);
	
	if (old_child == 0) {
		// Subdivide the node and create the child if, and only if, we are the first ones here.
		const uint child_level = level + 1;
		const uint child_size = voxel_node_size(child_level, voxel_P);

		// Allocate memory for child
		const uint child_ptr = atomicAdd(voxel_buffer_size, child_size);
		imageStore(voxels, voxels_image_coords(child_offset), child_ptr.xxxx);

		// Clear child
		const uint child_volatile_size = voxel_node_volatile_data_size(child_level, voxel_P);
		const uint volatile_data_ptr = child_ptr + voxel_node_children_offset(voxel_P);
		for (int u=0; u < child_volatile_size; ++u)
			imageStore(voxels, voxels_image_coords(volatile_data_ptr + u), uvec4(0));
			
		// Write user data to leafs
		if (child_level == voxel_leaf_level) {
			uint data_ptr = child_ptr + voxel_node_data_offset(child_level, voxel_P);
			imageStore(voxels, voxels_image_coords(data_ptr + 0), data.normal_roughness_packed.xxxx);
			imageStore(voxels, voxels_image_coords(data_ptr + 1), data.rgba.xxxx);
		}
	}

	// Calculate coordinates in brick
	v = fract(brick);
	// And index of pointer to child node
	node = child_offset;
}
