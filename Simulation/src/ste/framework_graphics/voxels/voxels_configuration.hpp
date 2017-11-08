//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

struct voxels_configuration {
	// Voxel structure end level index
	std::uint32_t leaf_level = 5;
	// Voxel world extent
	float world = 1000;

	//tex: $(2^P)^3$ voxels per block
	static constexpr std::uint32_t P = 1;

	// Size of voxel block
	auto tree_block_extent() const {
		return glm::vec3(static_cast<float>(1 << P));
	}

	// Resolution of maximal voxel level
	auto grid_resolution() const {
		return world / static_cast<float>(1 << P * leaf_level);
	}

	/**
	*	@brief	Returns the count of children in a node.
	*			Meaningless for leaf nodes.
	*/
	std::uint32_t node_children_count() const {
		return 1u << 3 * P;
	}

	/**
	*	@brief	Returns the offset of the custom data in a voxel node.
	*/
	std::uint32_t voxel_node_binary_map_offset() const {
		return 0u;
	}

	/**
	*	@brief	Returns the offset of the children data in a voxel node.
	*/
	std::uint32_t node_children_offset() const {
		return 1u;
	}

	/**
	*	@brief	Returns the size of a voxel node.
	*			level must be > 0.
	*/
	std::uint32_t node_size(std::uint32_t level) const {
		return node_children_offset() + glm::mix(node_children_count(), 1u, level == leaf_level - 1);
	}
};

}
}
