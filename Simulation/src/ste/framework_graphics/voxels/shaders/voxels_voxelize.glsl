
#include <voxels.glsl>


layout(r32ui, set=1, binding=2) restrict uniform uimage2D voxels;				// R32 uint

layout(std430, set=1, binding=6) restrict buffer voxels_counter_binding { uint voxel_buffer_size; };
layout(std430, set=1, binding=7) restrict buffer bricks_counter_binding { uint brick_image_size; };


/**
*	@brief	Atomically blends voxel's data
*/
void voxel_voxelize_blend_user_data(uint brick_ptr,
									vec3 brick,
									voxel_data_t data) {
	// Unpack input user data
	vec3 normal;
	float roughness;
	vec3 albedo;
	float opacity;
	float ior;
	float metallicity;
	decode_voxel_data(data, albedo, normal, roughness, opacity, ior, metallicity);

	// Blend albedo
	uint old_albedo_packed = 0;
	uint new_albedo_packed = data.packed[0];
	while ((new_albedo_packed = imageAtomicCompSwap(voxels, 
													voxels_image_coords(brick_ptr + 0),
													old_albedo_packed,
													new_albedo_packed)) != old_albedo_packed) {
		old_albedo_packed = new_albedo_packed;

		// Incerement counter
		const uint counter = (old_albedo_packed >> 24) + 1;
		// Blend
		const vec3 dst_albedo = mix(decode_voxel_data_albedo(old_albedo_packed), albedo, 1.f / float(counter));

		new_albedo_packed = encode_voxel_data_albedo(dst_albedo, counter);
	}
	
	// Blend normal, ior
	uint old_normal_packed = 0;
	uint new_normal_packed = data.packed[1];
	while ((new_normal_packed = imageAtomicCompSwap(voxels, 
													voxels_image_coords(brick_ptr + 1),
													old_normal_packed,
													new_normal_packed)) != old_normal_packed) {
		old_normal_packed = new_normal_packed;

		// Incerement counter
		const uint counter = (old_normal_packed >> 24) + 1;
		// Blend
		const float f = 1.f / float(counter);
		const vec3 dst_normal = normalize(mix(decode_voxel_data_normal(old_normal_packed), normal, f));
		const float dst_ior = mix(decode_voxel_data_ior(old_normal_packed), ior, f);

		new_normal_packed = encode_voxel_data_normal_ior(dst_normal, dst_ior, counter);
	}

	// Blend metadata
	uint old_metadata_packed = 0;
	uint new_metadata_packed = data.packed[2];
	while ((new_metadata_packed = imageAtomicCompSwap(voxels, 
													  voxels_image_coords(brick_ptr + 2),
													  old_metadata_packed,
													  new_metadata_packed)) != old_metadata_packed) {
		old_metadata_packed = new_metadata_packed;

		// Incerement counter
		const uint counter = (old_metadata_packed >> 24) + 1;
		// Blend
		const float f = 1.f / float(counter);
		const float dst_opacity = mix(decode_voxel_data_opacity(old_metadata_packed), opacity, f);
		const float dst_roughness = mix(decode_voxel_data_roughness(old_metadata_packed), roughness, f);
		const float dst_metallicity = mix(decode_voxel_data_metallicity(old_metadata_packed), metallicity, f);

		new_metadata_packed = encode_voxel_data_metadata(dst_opacity, dst_roughness, dst_metallicity, counter);
	}
}

void voxel_atomic_set_bit(uint node, uint bit) {
	const uint v = imageLoad(voxels, voxels_image_coords(node)).x;
	const uint m = 1 << bit;

	if ((v & m) == 0) {
		// Atomically set bit
		uint old_value;
		uint comp = v;
		while ((old_value = imageAtomicCompSwap(voxels, voxels_image_coords(node), 
												comp,
												comp | m)) != comp &&
			   (old_value & m) == 0) { 
			comp = old_value;
		}
	}
}

/**
*	@brief	Traverses a node, atomically creating a child at supplied position.
*/
uint voxel_voxelize(uint node, 
					uvec3 brick,
					uint level,
					out bool subdivided) {
	const uint child_semaphore_lock = 0xFFFFFFFF;
		
	// Calculate child index in block
	const uint child_idx = voxel_brick_index(brick);
	const uint child_offset = node + voxel_node_children_offset() + child_idx;

	// Attempt to acquire child semaphore lock
	const uint old_child = imageAtomicCompSwap(voxels,
											   voxels_image_coords(child_offset),
											   0,
											   child_semaphore_lock);
	
	// Subdivide the node and create the child if, and only if, we have lock. Otherwise, we are done.
	subdivided = old_child == 0;
	if (subdivided) {
		const uint child_level = level + 1;

		// For nodes:
		// Allocate memory for child, and write child pointer
		const uint child_size = voxel_node_size(child_level);
		const uint child_ptr = atomicAdd(voxel_buffer_size, child_size);
		imageAtomicExchange(voxels, voxels_image_coords(child_offset), child_ptr);
		
		// Clear child 
		const uint child_volatile_size = voxel_node_volatile_data_size(child_level); 
		const uint volatile_data_ptr = child_ptr + voxel_node_volatile_data_offset();
		for (int u=0; u < child_volatile_size; ++u)
			imageStore(voxels, voxels_image_coords(volatile_data_ptr + u), uvec4(0));

		// Atomically set occupancy bit
		voxel_atomic_set_bit(node + voxel_node_binary_map_offset(), child_idx);
	}

	// Return pointer to child node
	return child_offset;
}
