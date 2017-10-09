
// (2^P)^3 voxels per block
layout(constant_id=0) const uint voxel_P = 2;
// (2^Pi)^3 voxels per initial block
layout(constant_id=1) const uint voxel_Pi = 4;
// Voxel structure end level index
layout(constant_id=2) const uint voxel_leaf_level = 5;

// Voxel world size
layout(constant_id=3) const float voxel_world = 1000;

// Size of voxel block
const uvec3 voxel_block_size = uvec3(1 << voxel_P);
const uvec3 voxel_initial_block_size = uvec3(1 << voxel_Pi);

// Resolution of maximal voxel level
const float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << (voxel_P * (voxel_levels - 1))));

const uint voxel_node_data_size = 8;
const uint voxel_leaf_data_size = 16;
const uint voxel_root_size = (1 << (3 * (voxel_Pi - 1))) * 33 + voxel_node_data_size;
const uint voxel_node_size = (1 << (3 * (voxel_P - 1))) * 33 + voxel_node_data_size;
const uint voxel_leaf_size = (1 << (3 * (voxel_P - 1))) + voxel_node_data_size;

layout(std430, set=1, binding=0) restrict buffer voxel_buffer_binding {
	uint voxel_buffer[];
};
layout(std430, set=1, binding=1) restrict buffer voxel_counter_binding {
	uint voxel_buffer_size;
};
