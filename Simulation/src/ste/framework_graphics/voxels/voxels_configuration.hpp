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

	static constexpr std::uint32_t tree_node_data_size = 0;
	static constexpr std::uint32_t tree_leaf_data_size = 16;

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
	std::uint32_t node_children_count(std::uint32_t level) const {
		return glm::mix(1u << 3 * P, 0u, level == leaf_level);
	}

	/**
	*	@brief	Returns the size of the user data in a voxel node.
	*/
	std::uint32_t node_user_data_size(std::uint32_t level) const {
		return glm::mix(tree_node_data_size, tree_leaf_data_size, level == leaf_level) >> 2;
	}

	/**
	*	@brief	Returns the offset of the custom data in a voxel node.
	*/
	std::uint32_t node_data_offset(std::uint32_t level) const {
		return 0u;
	}

	/**
	*	@brief	Returns the offset of the children data in a voxel node.
	*/
	std::uint32_t node_children_offset(std::uint32_t level) const {
		return 0u;
	}

	/**
	*	@brief	Returns the size of a voxel node.
	*			level must be > 0.
	*/
	std::uint32_t node_size(std::uint32_t level) const {
		return glm::mix(node_children_offset(level) + node_children_count(level),
						node_user_data_size(level),
						level == leaf_level);
	}
};

}
}
