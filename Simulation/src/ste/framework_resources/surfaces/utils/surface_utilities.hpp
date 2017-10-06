//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>

#include <ste_type_traits.hpp>

namespace ste {
namespace resource {

struct surface_utilities {
	/**
	*	@brief	Computes maximal amount of mipmap levels, i.e. the count of levels to create a complete mipmap-chain.
	*/
	template <typename extent_type>
	static constexpr levels_t max_levels(const extent_type &extent) {
		static constexpr auto dimensions = type_elements_count_v<extent_type>;
		static_assert(dimensions >= 1 && dimensions <= 3);

		auto max_comp = extent.x;
		if constexpr (dimensions > 1) max_comp = glm::max(max_comp, extent.y);
		if constexpr (dimensions > 2) max_comp = glm::max(max_comp, extent.z);

		return levels_t(static_cast<std::size_t>(glm::log2<float>(static_cast<float>(max_comp)) + 1));
	}

	/**
	*	@brief	Returns the extent size, in texels, of a level
	*
	*	@param	surface_extent	Surface extent
	*	@param	level			Surface level
	*/
	template <typename extent_type>
	static constexpr auto extent(const extent_type &surface_extent,
								 levels_t level = 0_mips) {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						surface_extent >> static_cast<typename extent_type::value_type>(level));
	}

	/**
	*	@brief	Returns the extent size, in texels, of a block
	*/
	template <gl::format format>
	static constexpr auto block_extent() {
		return gl::format_traits<format>::block_extent;
	}

	/**
	*	@brief	Returns the extent size, in blocks, of a level
	*
	*	@param	surface_extent	Surface extent
	*	@param	level			Surface level
	*/
	template <gl::format format, typename extent_type>
	static constexpr auto extent_in_blocks(const extent_type &surface_extent,
										   levels_t level = 0_mips) {
		static constexpr auto dimensions = type_elements_count_v<extent_type>;
		static_assert(dimensions >= 1 && dimensions <= 3);

		auto level_extent = extent(surface_extent, level);
		auto one = static_cast<typename extent_type::value_type>(1);

		extent_type blocks;
		if constexpr (dimensions > 0)	blocks[0] = glm::max(one, level_extent[0] / block_extent<format>()[0]);
		if constexpr (dimensions > 1)	blocks[1] = glm::max(one, level_extent[1] / block_extent<format>()[1]);
		if constexpr (dimensions > 2)	blocks[2] = level_extent[2];

		return blocks;
	}

	/**
	*	@brief	Returns the total block count in a single surface level
	*
	*	@param	surface_extent	Surface extent
	*	@param	level			Surface level
	*/
	template <gl::format format, typename extent_type>
	static constexpr auto blocks(const extent_type &surface_extent,
								 levels_t level) {
		static constexpr auto dimensions = type_elements_count_v<extent_type>;
		static_assert(dimensions >= 1 && dimensions <= 3);

		auto level_extent = extent(surface_extent, level);
		auto one = static_cast<typename extent_type::value_type>(1);

		std::size_t b = 1;
		if constexpr (dimensions > 0)	b *= glm::max(one, level_extent[0] / block_extent<format>()[0]);
		if constexpr (dimensions > 1)	b *= glm::max(one, level_extent[1] / block_extent<format>()[1]);
		if constexpr (dimensions > 2)	b *= level_extent[2];

		return b;
	}

	/**
	*	@brief	Returns the count of blocks that compose the whole mipmap chain of a layer
	*
	*	@param	surface_extent	Surface extent
	*	@param	levels			Surface levels count
	*/
	template <gl::format format, typename extent_type>
	static constexpr auto blocks_layer(const extent_type &surface_extent,
									   levels_t levels) {
		std::size_t b = 0;
		for (auto l = 0_mips; l < levels; ++l)
			b += blocks<format>(surface_extent, l);

		return b;
	}

	/**
	*	@brief	Computes the offset, in blocks, to a specified level of a specified layer of the surface
	*
	*	@param	surface_extent	Surface extent
	*	@param	levels			Surface levels count
	*	@param	layer			Surface layer
	*	@param	level			Surface level
	*/
	template <gl::format format, typename extent_type>
	static constexpr std::size_t offset_blocks(const extent_type &surface_extent,
											   levels_t levels,
											   layers_t layer,
											   levels_t level) {
		std::size_t level_offset = 0;
		for (auto l = 0_mips; l < level; ++l)
			level_offset += blocks<format>(surface_extent, l);

		return blocks_layer<format>(surface_extent, levels) * static_cast<std::size_t>(layer) + level_offset;
	}

	/**
	*	@brief	Returns the size, in bytes, of a block
	*/
	template <gl::format format>
	static constexpr auto block_bytes() {
		return gl::format_traits<format>::block_bytes;
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface data
	*
	*	@param	surface_extent	Surface extent
	*	@param	levels			Surface levels count
	*/
	template <gl::format format, typename extent_type>
	static constexpr auto bytes(const extent_type &surface_extent,
								levels_t levels,
								layers_t layers) {
		return block_bytes<format>() * blocks_layer<format>(surface_extent, levels) * static_cast<std::size_t>(layers);
	}

	/**
	*	@brief	Returns the size, in bytes, of a surface layer's level
	*	
	*	@param	level			Surface level
	*/
	template <gl::format format, typename extent_type>
	static constexpr auto bytes(const extent_type &surface_extent,
								levels_t level) {
		return block_bytes<format>() * blocks<format>(surface_extent, level);
	}
};

}
}
