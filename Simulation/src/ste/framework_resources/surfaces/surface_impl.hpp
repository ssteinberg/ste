// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>
#include <surface_common.hpp>

namespace ste {
namespace resource {

namespace _detail {

template<gl::format format, typename extent_type, typename block_ptr_type>
class surface_image {
private:
	extent_type extent;
	block_ptr_type storage;

	std::size_t _blocks;

public:
	surface_image(const extent_type &extent,
				  block_ptr_type storage,
				  std::size_t blocks)
		: extent(extent),
		storage(storage),
		_blocks(blocks)
	{}

	auto data() { return storage; }
	auto data() const { return storage; }

	auto& operator[](std::size_t block_index) {
		assert(block_index < _blocks);
		return storage[block_index];
	}
	auto& operator[](std::size_t block_index) const {
		assert(block_index < _blocks);
		return storage[block_index];
	}

	auto blocks() const { return _blocks; }
};

template<gl::format format, gl::image_type image_type, bool is_const = false>
class surface : public surface_base<format, image_type> {
	using Base = surface_base<format, image_type>;

public:
	using Base::extent_type;
	using Base::traits;

	using block_type = typename traits::element_type;

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

	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* data() { return storage.get(); }
	const block_type* data() const { return storage.get(); }

	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	auto operator[](std::size_t level_index) {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return layer_type(Base::extent(),
						  ptr,
						  Base::blocks(level_index));
	}
	auto operator[](std::size_t level_index) const {
		auto ptr = &data()[Base::offset_blocks(level_index, 0)];
		return const_layer_type(Base::extent(),
								ptr,
								Base::blocks(level_index));
	}
};

}

}
}
