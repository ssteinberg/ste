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
template <gl::format format, typename extent_type, typename block_ptr_type>
class surface_image {
private:
	extent_type extent;
	extent_type blockextent;
	block_ptr_type storage;

	std::size_t _blocks;

public:
	surface_image(const extent_type& extent,
	              const extent_type& block_extent,
	              block_ptr_type storage,
	              std::size_t blocks)
		: extent(extent),
		  blockextent(block_extent),
		  storage(storage),
		  _blocks(blocks) {
		// Sanity
		assert((extent.x / block_extent.x) * (extent.y / block_extent.y) == blocks);
		assert((extent.x % block_extent.x) == 0 && (extent.y % block_extent.y) == 0);
	}

	~surface_image() noexcept {}

	surface_image(surface_image&&) = default;
	surface_image(const surface_image&) = delete;
	surface_image& operator=(surface_image&&) = default;
	surface_image& operator=(const surface_image&) = delete;

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
	auto& at(const extent_type& block_coord) {
		const std::size_t block_index = block_coord.y * block_extent().x + block_coord.x;
		return (*this)[block_index];
	}

	/**
	*	@brief	Returns a reference to a block in the image
	*
	*	@param	block_coord		Coordinates of the block
	*/
	auto& at(const extent_type& block_coord) const {
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

template <gl::format format, gl::image_type image_type>
class const_surface : public surface_base<format, image_type> {
	using Base = surface_base<format, image_type>;

public:
	using Base::extent_type;
	using Base::block_type;
	using Base::traits;

	static_assert(sizeof(block_type) == traits::block_bytes, "sizeof(block_type) != block_bytes");
	using const_layer_type = surface_image<format, extent_type, const block_type*>;

protected:
	surface_storage<block_type> storage;

	const_surface(const extent_type& extent,
	              std::size_t levels)
		: Base(extent, levels, 1),
		  storage(Base::blocks_layer() * Base::layers()) {}

public:
	const_surface(const extent_type& extent,
	              std::size_t levels,
	              const surface_storage<block_type>& storage)
		: Base(extent, levels, 1),
		  storage(storage) {}

	~const_surface() noexcept {}

	const_surface(const_surface&&) = default;
	const_surface(const const_surface&) = delete;
	const_surface& operator=(const_surface&&) = default;
	const_surface& operator=(const const_surface&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	const block_type* data() const override final { return storage.get(); }

	/**
	*	@brief	Returns an image for the queried level index
	*
	*	@param	level_index		Level index
	*/
	const_layer_type operator[](std::size_t level_index) const {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return const_layer_type(Base::extent(),
		                        Base::block_extent(),
		                        ptr,
		                        Base::blocks(level_index));
	}
};

/**
*	@brief	Surface
*/
template <gl::format format, gl::image_type image_type>
class surface : public const_surface<format, image_type> {
	using Base = const_surface<format, image_type>;

public:
	using Base::extent_type;
	using Base::block_type;
	using Base::traits;

	using layer_type = surface_image<format, extent_type, block_type*>;

public:
	surface(const extent_type& extent,
	        std::size_t levels = 1)
		: Base(extent, levels) {}

	// For signature compatability with surface_array
	surface(const extent_type& extent,
			std::size_t layers,
			std::size_t levels)
		: Base(extent, levels) {
		assert(layers == 1);
	}

	surface(const extent_type& extent,
	        std::size_t levels,
	        const surface_storage<block_type>& storage)
		: Base(extent, levels, storage) {}

	~surface() noexcept {}

	surface(surface&&) = default;
	surface& operator=(surface&&) = default;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	block_type* data() override final { return Base::storage.get(); }
	using Base::data;

	/**
	*	@brief	Returns an image for the queried level index
	*	
	*	@param	level_index		Level index
	*/
	layer_type operator[](std::size_t level_index) {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return layer_type(Base::extent(),
		                  Base::block_extent(),
		                  ptr,
		                  Base::blocks(level_index));
	}

	using Base::operator[];
};

}

}
}
