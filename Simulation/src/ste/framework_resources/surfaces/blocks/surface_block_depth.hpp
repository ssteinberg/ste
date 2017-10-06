//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

namespace ste {
namespace resource {

/**
*	@brief	Depth only block
*/
template <block_type type, unsigned d_bits, unsigned d_offset_bits = 0>
class block_depth {
	using common_type_selector_t = _detail::block_common_type_selector<type, d_bits>;

public:
	static constexpr unsigned elements = 1;
	static constexpr block_type blocktype = type;

	static constexpr unsigned d_index = 0;
	static constexpr unsigned d_offset = d_offset_bits;

	template <gl::component_swizzle c>
	static constexpr int index_for_component() {
		if constexpr (c == gl::component_swizzle::d) return d_index;
		static_assert(c == gl::component_swizzle::d);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int offset_for_component() {
		if constexpr (c == gl::component_swizzle::d) return d_offset;
		static_assert(c == gl::component_swizzle::d);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int size_for_component() {
		if constexpr (c == gl::component_swizzle::d) return d_bits;
		static_assert(c == gl::component_swizzle::d);
		return -1;
	}

	static constexpr unsigned total_bits = d_bits + d_offset;
	static constexpr byte_t bytes = byte_t(total_bits) >> 3;

	using d_comp_type = typename _detail::block_primary_type_selector<type, d_bits>::type;
	using d_comp_writer_type = typename _detail::block_primary_type_selector<type, d_bits>::block_writer_type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

	template <gl::component_swizzle c>
	using comp_type = d_comp_type;
	template <gl::component_swizzle c>
	using comp_writer_type = d_comp_writer_type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0_B, "bytes is zero");

	using data_t = std::uint8_t[static_cast<std::size_t>(bytes)];

	data_t data{};

public:
	block_depth() = default;
	block_depth(d_comp_type in_d) {
		d() = in_d;
	}

	/**
	*	@brief 	Reads depth component's value
	*/
	d_comp_type d() const { return _detail::block_component<type, d_bits, d_offset, true>(data); }

	/**
	*	@brief 	Provides read-write access to depth component
	*/
	auto d() { return _detail::block_component<type, d_bits, d_offset, false>(data); }

	template <int index>
	auto component() {
		static_assert(index >= 0 && index <= 0);
		return d();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 0);
		return d();
	}
};

}
}
