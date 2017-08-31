//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>
#include <surface_common.hpp>

#include <surface_block.hpp>

namespace ste {
namespace resource {

namespace _detail {

/**
*	@brief	A surface image, which represents a level of a layer of a surface.
*/
template<gl::format format, typename extent_type, typename block_ptr_type>
class surface_image {
private:
	extent_type extent;
	extent_type blockextent;
	block_ptr_type storage;

	std::size_t _blocks;

public:
	surface_image(const extent_type &extent,
				  const extent_type &block_extent,
				  block_ptr_type storage,
				  std::size_t blocks)
		: extent(extent),
		blockextent(block_extent),
		storage(storage),
		_blocks(blocks) 
	{
		// Sanity
		assert((extent.x / block_extent.x) * (extent.y / block_extent.y) == blocks);
		assert((extent.x % block_extent.x) == 0 && (extent.y % block_extent.y) == 0);
	}
	~surface_image() noexcept {}

	surface_image(surface_image&&) = default;
	surface_image(const surface_image&) = delete;
	surface_image &operator=(surface_image&&) = default;
	surface_image &operator=(const surface_image&) = delete;

	/**
	*	@brief	Returns a pointer to the image data
	*/
	auto data() { return storage; }
	/**
	*	@brief	Returns a pointer to the image data
	*/
	auto data() const { return storage; }

	/**
	*	@brief	Returns a reference to a block in the image
	*
	*	@param	block_index		Address of the block
	*/
	auto& operator[](std::size_t block_index) {
		assert(block_index < _blocks);
		return storage[block_index];
	}
	/**
	*	@brief	Returns a reference to a block in the image
	*
	*	@param	block_index		Address of the block
	*/
	auto& operator[](std::size_t block_index) const {
		assert(block_index < _blocks);
		return storage[block_index];
	}

	/**
	*	@brief	Returns a reference to a block in the image
	*
	*	@param	block_coord		Coordinates of the block
	*/
	auto& at(const extent_type &block_coord) {
		const std::size_t block_index = block_coord.y * block_extent().x + block_coord.x;
		return (*this)[block_index];
	}
	/**
	*	@brief	Returns a reference to a block in the image
	*
	*	@param	block_coord		Coordinates of the block
	*/
	auto& at(const extent_type &block_coord) const {
		const std::size_t block_index = block_coord.y * block_extent().x + block_coord.x;
		return (*this)[block_index];
	}

	/**
	*	@brief	Returns block count in the image
	*/
	auto blocks() const { return _blocks; }
	/**
	*	@brief	Returns the extent size of a block
	*/
	auto block_extent() const {
		return blockextent;
	}
};

/**
*	@brief	Surface
*/
template<gl::format format, gl::image_type image_type, bool is_const = false>
class surface : public surface_base<format, image_type> {
	using Base = surface_base<format, image_type>;

public:
	using Base::extent_type;
	using Base::traits;

	using block_type = typename traits::block_type;
	static_assert(sizeof(block_type) == traits::block_bytes, "sizeof(block_type) != block_bytes");

	using layer_type = surface_image<format, extent_type, block_type*>;
	using const_layer_type = surface_image<format, extent_type, const block_type*>;

private:
	surface_storage<block_type, is_const> storage;

public:
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	surface(const extent_type &extent,
			std::size_t levels = 1)
		: Base(extent, levels, 1),
		storage(Base::blocks_layer() * Base::layers())
	{}
	surface(const extent_type &extent,
			std::size_t levels,
			const surface_storage<block_type, is_const> &storage)
		: Base(extent, levels, 1),
		storage(storage)
	{}
	~surface() noexcept {}

	surface(surface&&) = default;
	surface(const surface&) = delete;
	surface &operator=(surface&&) = default;
	surface &operator=(const surface&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* data() { return storage.get(); }
	/**
	*	@brief	Returns a pointer to the surface data
	*/
	const block_type* data() const { return storage.get(); }

	/**
	*	@brief	Returns an image for the queried level index
	*	
	*	@param	level_index		Level index
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	auto operator[](std::size_t level_index) {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return layer_type(Base::extent(),
						  Base::block_extent(),
						  ptr,
						  Base::blocks(level_index));
	}
	/**
	*	@brief	Returns an image for the queried level index
	*
	*	@param	level_index		Level index
	*/
	auto operator[](std::size_t level_index) const {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return const_layer_type(Base::extent(),
								Base::block_extent(),
								ptr,
								Base::blocks(level_index));
	}
};

}

}
}
