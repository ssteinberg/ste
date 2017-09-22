//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

namespace ste {
namespace resource {

/**
*	@brief	2-component block
*/
template <block_type type, unsigned comp0_bits, unsigned comp1_bits, gl::component_swizzle comp0, gl::component_swizzle comp1>
class block_2components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g);

	using common_type_selector_t = _detail::block_common_type_selector<type, std::max(comp0_bits, comp1_bits)>;

public:
	static constexpr unsigned elements = 2;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 : 1;
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 : 1;
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits : comp1_bits;
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits : comp1_bits;
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 : comp0_bits;
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 : comp0_bits;

	template <gl::component_swizzle c>
	static constexpr int index_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int offset_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_offset;
		if constexpr (c == gl::component_swizzle::g) return g_offset;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int size_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_bits;
		if constexpr (c == gl::component_swizzle::g) return g_bits;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using r_comp_writer_type = typename _detail::block_primary_type_selector<type, r_bits>::block_writer_type;
	using g_comp_writer_type = typename _detail::block_primary_type_selector<type, g_bits>::block_writer_type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

	template <gl::component_swizzle c>
	using comp_type = std::conditional_t<c == gl::component_swizzle::r, r_comp_type, g_comp_type>;
	template <gl::component_swizzle c>
	using comp_writer_type = std::conditional_t<c == gl::component_swizzle::r, r_comp_writer_type, g_comp_writer_type>;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_2components() = default;
	block_2components(r_comp_type in_r, g_comp_type in_g) {
		r() = in_r;
		g() = in_g;
	}

	/**
	*	@brief 	Reads red (1st) component's value
	*/
	r_comp_type r() const { return _detail::block_component<type, r_bits, r_offset, true>(data); }
	/**
	*	@brief 	Reads green (2nd) component's value
	*/
	g_comp_type g() const { return _detail::block_component<type, g_bits, g_offset, true>(data); }

	/**
	*	@brief 	Provides read-write access to red (1st) component
	*/
	auto r() { return _detail::block_component<type, r_bits, r_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to green (2nd) component
	*/
	auto g() { return _detail::block_component<type, g_bits, g_offset, false>(data); }

	template <int index>
	auto component() {
		static_assert(index >= 0 && index <= 1);
		if constexpr (index == r_index) return r();
		if constexpr (index == g_index) return g();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 1);
		if constexpr (index == r_index) return r();
		if constexpr (index == g_index) return g();
	}
};

}
}
