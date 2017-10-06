//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

namespace ste {
namespace resource {

/**
*	@brief	Single-component block
*/
template <block_type type, unsigned comp0_bits>
class block_1components {
	using common_type_selector_t = _detail::block_common_type_selector<type, comp0_bits>;

public:
	static constexpr unsigned elements = 1;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = 0;
	static constexpr unsigned r_bits = comp0_bits;
	static constexpr unsigned r_offset = 0;

	template <gl::component_swizzle c>
	static constexpr int index_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		static_assert(c == gl::component_swizzle::r);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int offset_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_offset;
		static_assert(c == gl::component_swizzle::r);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int size_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_bits;
		static_assert(c == gl::component_swizzle::r);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits;
	static constexpr byte_t bytes = byte_t(total_bits) >> 3;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using r_comp_writer_type = typename _detail::block_primary_type_selector<type, r_bits>::block_writer_type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

	template <gl::component_swizzle c>
	using comp_type = r_comp_type;
	template <gl::component_swizzle c>
	using comp_writer_type = r_comp_writer_type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0_B, "bytes is zero");

	using data_t = std::uint8_t[static_cast<std::size_t>(bytes)];

	data_t data{};

public:
	block_1components() = default;
	block_1components(r_comp_type in_r) {
		r() = in_r;
	}

	/**
	*	@brief 	Reads red (1st) component's value
	*/
	r_comp_type r() const { return _detail::block_component<type, r_bits, r_offset, true>(data); }

	/**
	*	@brief 	Provides read-write access to red (1st) component
	*/
	auto r() { return _detail::block_component<type, r_bits, r_offset, false>(data); }

	template <int index>
	auto component() {
		static_assert(index >= 0 && index <= 0);
		return r();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 0);
		return r();
	}
};

}
}
