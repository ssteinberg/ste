//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_utilities.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <memory>

namespace ste {
namespace resource {

namespace _detail {

/**
 *	@brief	Surface shared storage
 */
template<typename block_type>
class surface_storage {
private:
	std::shared_ptr<block_type> data;
	std::size_t offset;

public:
	surface_storage(const std::shared_ptr<block_type> &data,
					std::size_t offset = 0)
		: data(data), offset(offset)
	{}
	surface_storage(std::shared_ptr<block_type> &&data,
					std::size_t offset = 0)
		: data(std::move(data)), offset(offset)
	{}
	surface_storage(std::size_t blocks)
		: data(new block_type[blocks],
			   std::default_delete<block_type[]>()),
		offset(0)
	{}

	surface_storage(surface_storage&&) = default;
	surface_storage(const surface_storage&) = default;
	surface_storage &operator=(surface_storage&&) = default;
	surface_storage &operator=(const surface_storage&) = default;

	auto view(std::size_t offset) const { return surface_storage(data, offset); }

	block_type* get() { return data.get() + offset; }
	const block_type* get() const { return data.get() + offset; }
};

/**
*	@brief	Surface base class
*/
template<gl::format format, gl::image_type image_type>
class surface_base {
public:
	using traits = gl::format_traits<format>;
	static constexpr auto dimensions = gl::image_dimensions_v<image_type>;
	using extent_type = typename gl::image_extent_type<dimensions>::type;
	using block_type = typename traits::block_type;

private:
	extent_type surface_extent;
	levels_t surface_levels;
	layers_t surface_layers;

protected:
	surface_base(const extent_type &extent,
				 levels_t levels,
				 layers_t layers)
		: surface_extent(extent), surface_levels(levels), surface_layers(layers)
	{
		assert(layers > 0_layers);
		assert(levels > 0_mips);
	}

public:
	virtual ~surface_base() noexcept {}

	surface_base(surface_base&&) = default;
	surface_base(const surface_base&) = delete;
	surface_base &operator=(surface_base&&) = default;
	surface_base &operator=(const surface_base&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	virtual block_type* data() = 0;
	/**
	*	@brief	Returns a pointer to the surface data
	*/
	virtual const block_type* data() const = 0;

	/**
	*	@brief	Returns a pointer to the surface layer's level data
	*	
	*	@param	layer			Surface layer
	*	@param	level			Surface level
	*/
	block_type* data_at(layers_t layer, levels_t level = 0_mips) {
		return data() + offset_blocks(layer, level);
	}
	/**
	*	@brief	Returns a pointer to the surface layer's level data
	*	
	*	@param	layer			Surface layer
	*	@param	level			Surface level
	*/
	const block_type* data_at(layers_t layer, levels_t level = 0_mips) const {
		return data() + offset_blocks(layer, level);
	}

	/**
	*	@brief	Returns the extent size, in texels, of a level
	*
	*	@param	level	Surface level
	*/
	auto extent(levels_t level = 0_mips) const {
		return surface_utilities::extent(surface_extent,
										 level);
	}
	/**
	*	@brief	Returns the extent size, in blocks, of a level
	*
	*	@param	level	Surface level
	*/
	auto extent_in_blocks(levels_t level = 0_mips) const {
		return surface_utilities::extent_in_blocks<format>(surface_extent,
														   level);
	}
	/**
	*	@brief	Returns the levels count in the surface
	*/
	auto levels() const {
		return surface_levels;
	}
	/**
	*	@brief	Returns the layers count in the surface
	*/
	auto layers() const {
		return surface_layers;
	}
	/**
	*	@brief	Returns the surface image type
	*/
	static constexpr auto surface_image_type() { return image_type; }
	/**
	*	@brief	Returns the surface format
	*/
	static constexpr auto surface_format() { return format; }
	/**
	*	@brief	Returns the surface's dimensions count
	*/
	static constexpr auto surface_dimensions() { return dimensions; }

	/**
	*	@brief	Returns the extent size of a block
	*/
	static constexpr auto block_extent() {
		return gl::format_traits<format>::block_extent;
	}

	/**
	*	@brief	Returns the block count in a single surface level
	*/
	auto blocks(levels_t level) const {
		return surface_utilities::blocks<format>(surface_extent, 
												 level);
	}

	/**
	*	@brief	Returns the count of blocks that compose the whole mipmap chain of a layer
	*/
	auto blocks_layer() const {
		return surface_utilities::blocks_layer<format>(surface_extent, 
													   levels());
	}

	/**
	*	@brief	Computes the offset, in blocks, to a specified level of a specified layer of the surface
	*	
	*	@param	layer			Surface layer
	*	@param	level			Surface level
	*/
	std::size_t offset_blocks(layers_t layer, levels_t level) const {
		return surface_utilities::offset_blocks<format>(surface_extent, 
														levels(),
														layer,
														level);
	}

	/**
	*	@brief	Returns the size, in bytes, of a block
	*/
	static constexpr auto block_bytes() {
		return surface_utilities::block_bytes<format>();
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface data
	*/
	auto bytes() const {
		return surface_utilities::bytes<format>(surface_extent,
												levels(),
												layers());
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface layer's level
	*	
	*	@param	level			Surface level
	*/
	auto bytes(levels_t level) const {
		return surface_utilities::bytes<format>(surface_extent,
												level);
	}

	static constexpr auto get_format_traits() { return traits(); }
};

}

}
}
