//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>

#include <surface_impl.hpp>
#include <surface_cubemap.hpp>

namespace ste {
namespace resource {

namespace _detail {

/**
*	@brief	Surface array
*/
template <gl::format format, gl::image_type image_type>
class surface_array : public surface_base<format, image_type> {
	using Base = surface_base<format, image_type>;

public:
	using surface_layer_type = std::conditional_t<
		image_type == gl::image_type::image_cubemap_array,
		surface_cubemap<format>,
		surface<format, gl::image_layer_type_v<image_type>>
	>;

	using Base::extent_type;
	using Base::block_type;
	using Base::traits;

	static_assert(sizeof(block_type) == traits::block_bytes, "sizeof(block_type) != block_bytes");

	using layer_type = surface_image<format, extent_type, block_type*>;
	using const_layer_type = surface_image<format, extent_type, const block_type*>;

private:
	surface_storage<block_type> storage;

public:
	surface_array(const extent_type& extent,
	              std::size_t layers,
	              std::size_t levels = 1)
		: Base(extent, levels, layers),
		  storage(Base::blocks_layer() * Base::layers()) {}

	~surface_array() noexcept {}

	surface_array(surface_array&&) = default;
	surface_array(const surface_array&) = delete;
	surface_array& operator=(surface_array&&) = default;
	surface_array& operator=(const surface_array&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	block_type* data() override final { return storage.get(); }

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	const block_type* data() const override final { return storage.get(); }

	/**
	*	@brief	Returns a non-array surface for the queried layer index
	*
	*	@param	layer_index		Layer index
	*/
	auto operator[](std::size_t layer_index) {
		assert(layer_index < Base::layers());
		return surface_layer_type(Base::extent(),
		                          Base::levels(),
		                          storage.view(Base::offset_blocks(layer_index, 0)));
	}

	/**
	*	@brief	Returns a non-array surface for the queried layer index
	*
	*	@param	layer_index		Layer index
	*/
	auto operator[](std::size_t layer_index) const {
		assert(layer_index < Base::layers());
		return surface_layer_type(Base::extent(),
		                          Base::levels(),
		                          storage.view(Base::offset_blocks(layer_index, 0)));
	}
};

}

}
}
