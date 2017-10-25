//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

struct voxels_configuration {
	//tex: $(2^P)^3$ voxels per block
	std::uint32_t P = 2;
	//tex: $(2^{P_i})^3$ voxels per initial block
	std::uint32_t Pi = 4;
	// Voxel structure end level index
	std::uint32_t leaf_level = 5;

	// Voxel world extent
	float world = 1000;

	static constexpr std::uint32_t voxel_tree_node_data_size = 8;
	static constexpr std::uint32_t voxel_tree_leaf_data_size = 8;

	// Size of voxel block
	auto voxel_tree_block_extent() const {
		return glm::vec3(static_cast<float>(1 << P));
	}
	auto voxel_tree_initial_block_extent() const {
		return glm::vec3(static_cast<float>(1 << Pi));
	}

	// Resolution of maximal voxel level
	auto voxel_grid_resolution() const {
		return world / static_cast<float>((1 << Pi) * (1 << P * (leaf_level - 1)));
	}

	auto voxel_tree_root_binary_map_size() const {
		return byte_t(1 << 3 * (Pi - 1));
	}
	auto voxel_tree_root_volatile_data_size() const {
		return byte_t(1 << 3 * (Pi - 1));
	}
	auto voxel_tree_root_size() const {
		return byte_t((1 << 3 * (Pi - 1)) * 33 + voxel_tree_node_data_size);
	}
	auto voxel_tree_node_size() const {
		return byte_t((1 << 3 * (P - 1)) * 33 + voxel_tree_node_data_size);
	}
	auto voxel_tree_leaf_size() const {
		return byte_t(voxel_tree_leaf_data_size);
	}

	template <int Pi>
	struct tree_root_t {
		static constexpr auto bricks_count = 1 << 3 * Pi;

		std::uint32_t binary_map[bricks_count / 32];
		std::uint32_t children[bricks_count];
	};
	template <int P>
	struct tree_node_t {
		static constexpr auto bricks_count = 1 << 3 * P;

		std::uint32_t binary_map[bricks_count / 32];
		std::uint32_t children[bricks_count];
	};
};

}
}
