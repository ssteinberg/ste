// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>

#include <surface_impl.hpp>
#include <surface_cubemap.hpp>

namespace ste {
namespace resource {

namespace _detail {

template<gl::format format, gl::image_type image_type>
class surface_array : public surface_base<format, image_type> {
	using Base = surface_base<format, image_type>;

public:
	using surface_layer_type = std::conditional_t<
		image_type == gl::image_type::image_cubemap_array,
		surface_cubemap<format>,
		surface<format, gl::image_layer_type_v<image_type>>
	>;

	using Base::extent_type;
	using Base::traits;

	using block_type = typename traits::element_type;

	using layer_type = surface_image<format, extent_type, block_type*>;
	using const_layer_type = surface_image<format, extent_type, const block_type*>;

private:
	surface_storage<block_type, false> storage;

public:
	surface_array(const extent_type &extent,
				  std::size_t layers,
				  std::size_t levels = 1)
		: Base(extent, levels, layers),
		storage(Base::blocks_layer() * Base::layers())
	{}

	auto* data() { return storage.get(); }
	auto* data() const { return storage.get(); }

	auto operator[](std::size_t layer_index) {
		assert(layer_index < Base::layers());
		return surface<format, image_type, false>(Base::extent(),
												  Base::levels(),
												  storage.view(Base::offset_blocks(layer_index, 0)));
	}
	auto operator[](std::size_t layer_index) const {
		assert(layer_index < Base::layers());
		return surface<format, image_type, true>(Base::extent(),
												 Base::levels(),
												 storage.view(Base::offset_blocks(layer_index, 0)));
	}
};

}

}
}
