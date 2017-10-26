
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


/**
*	@brief	Atomically blends voxel's data
*/
void voxel_voxelize_blend_user_data(uint node,
									uint level,
									voxel_data_t data) {
	const uint data_ptr = node + voxel_node_data_offset(level, voxel_P);

	// Unpack input user data
	vec3 normal;
	float roughness;
	vec3 albedo;
	float opacity;
	decode_voxel_data(data, normal, roughness, albedo, opacity);

	// Blend albedo
	uint old_albedo_packed = 0;
	uint new_albedo_packed = data.albedo_packed;
	while ((new_albedo_packed = imageAtomicCompSwap(voxels, 
													voxels_image_coords(data_ptr), 
													old_albedo_packed,
													new_albedo_packed)) != old_albedo_packed) {
		old_albedo_packed = new_albedo_packed;

		// Incerement counter
		const uint counter = (old_albedo_packed >> 24) + 1;
		// Blend
		const vec3 dst_albedo = mix(decode_voxel_data_albedo(old_albedo_packed), albedo, 1.f / float(counter));

		new_albedo_packed = encode_voxel_data_albedo(dst_albedo, counter);
	}

	// Blend normal
	uint old_normal_packed = 0;
	uint new_normal_packed = data.normal_packed;
	while ((new_normal_packed = imageAtomicCompSwap(voxels, 
													voxels_image_coords(data_ptr + 1), 
													old_normal_packed,
													new_normal_packed)) != old_normal_packed) {
		old_normal_packed = new_normal_packed;

		// Incerement counter
		const uint counter = (old_normal_packed >> 24) + 1;
		// Blend
		const vec3 dst_normal = normalize(mix(decode_voxel_data_normal(old_normal_packed), normal, 1.f / float(counter)));

		new_normal_packed = encode_voxel_data_normal(dst_normal, counter);
	}

	// Blend rougness and opacity
	uint old_opacity_roughness_packed = 0;
	uint new_opacity_roughness_packed = data.opacity_roughness_packed;
	while ((new_opacity_roughness_packed = imageAtomicCompSwap(voxels, 
															   voxels_image_coords(data_ptr + 2),
															   old_opacity_roughness_packed,
															   new_opacity_roughness_packed)) != old_opacity_roughness_packed) {
		old_opacity_roughness_packed = new_opacity_roughness_packed;

		// Incerement counter
		const uint counter = (old_opacity_roughness_packed >> 24) + 1;
		// Blend
		const float f = 1.f / float(counter);
		const float dst_opacity = mix(decode_voxel_data_opacity(old_opacity_roughness_packed), opacity, f);
		const float dst_roughness = mix(decode_voxel_data_roughness(old_opacity_roughness_packed), roughness, f);

		new_opacity_roughness_packed = encode_voxel_data_opacity_roughness(dst_opacity, dst_roughness, counter);
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
	const uint child_offset = node + voxel_node_children_offset(level, P) + child_idx;

	// Attempt to acquire child semaphore lock
	const uint old_child = imageAtomicCompSwap(voxels, 
											   voxels_image_coords(child_offset),
											   0,
											   child_semaphore_lock);
	// Subdivide the node and create the child if, and only if, we have lock. Otherwise, we are done.
	if (old_child == 0) {
		const uint child_level = level + 1;
		const uint child_size = voxel_node_size(child_level, voxel_P);

		// Allocate memory for child
		const uint child_ptr = atomicAdd(voxel_buffer_size, child_size);
		imageAtomicExchange(voxels, voxels_image_coords(child_offset), child_ptr);

		// Increment occupancy counter of parent
		const uint occupancy_offset = node + voxel_node_occupancy_offset(level, P);
		imageAtomicAdd(voxels, voxels_image_coords(occupancy_offset), 1);
		
		// Clear child 
		const uint child_volatile_size = voxel_node_volatile_data_size(child_level, voxel_P); 
		const uint volatile_data_ptr = child_ptr + voxel_node_volatile_data_offset(child_level, voxel_P); 
		for (int u=0; u < child_volatile_size; ++u) 
			imageStore(voxels, voxels_image_coords(volatile_data_ptr + u), uvec4(0)); 
	}

	// Calculate coordinates in brick
	v = fract(brick);
	// And index of pointer to child node
	node = child_offset;
}
