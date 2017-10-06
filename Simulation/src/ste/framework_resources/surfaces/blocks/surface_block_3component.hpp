//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

namespace ste {
namespace resource {

/**
*	@brief	3-component block
*/
template <block_type type, unsigned comp0_bits, unsigned comp1_bits, unsigned comp2_bits, gl::component_swizzle comp0, gl::component_swizzle comp1, gl::component_swizzle comp2>
class block_3components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r || comp2 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g || comp2 == gl::component_swizzle::g);
	static_assert(comp0 == gl::component_swizzle::b || comp1 == gl::component_swizzle::b || comp2 == gl::component_swizzle::b);

	using common_type_selector_t = _detail::block_common_type_selector<type, std::max(std::max(comp0_bits, comp1_bits), comp2_bits)>;

public:
	static constexpr unsigned elements = 3;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? 1 : 2);
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? 1 : 2);
	static constexpr unsigned b_index = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? 1 : 2);
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits :
		(comp1 == gl::component_swizzle::r ? comp1_bits : comp2_bits);
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits :
		(comp1 == gl::component_swizzle::g ? comp1_bits : comp2_bits);
	static constexpr unsigned b_bits = comp0 == gl::component_swizzle::b ? comp0_bits :
		(comp1 == gl::component_swizzle::b ? comp1_bits : comp2_bits);
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? comp0_bits : comp0_bits + comp1_bits);
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? comp0_bits : comp0_bits + comp1_bits);
	static constexpr unsigned b_offset = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? comp0_bits : comp0_bits + comp1_bits);

	template <gl::component_swizzle c>
	static constexpr int index_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		if constexpr (c == gl::component_swizzle::b) return b_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int offset_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_offset;
		if constexpr (c == gl::component_swizzle::g) return g_offset;
		if constexpr (c == gl::component_swizzle::b) return b_offset;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b);
		return -1;
	}
	template <gl::component_swizzle c>
	static constexpr int size_for_component() {
		if constexpr (c == gl::component_swizzle::r) return r_bits;
		if constexpr (c == gl::component_swizzle::g) return g_bits;
		if constexpr (c == gl::component_swizzle::b) return b_bits;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits + comp2_bits;
	static constexpr byte_t bytes = byte_t(total_bits) >> 3;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using b_comp_type = typename _detail::block_primary_type_selector<type, b_bits>::type;
	using r_comp_writer_type = typename _detail::block_primary_type_selector<type, r_bits>::block_writer_type;
	using g_comp_writer_type = typename _detail::block_primary_type_selector<type, g_bits>::block_writer_type;
	using b_comp_writer_type = typename _detail::block_primary_type_selector<type, b_bits>::block_writer_type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

	template <gl::component_swizzle c>
	using comp_type = std::conditional_t<c == gl::component_swizzle::r, r_comp_type,
		std::conditional_t<c == gl::component_swizzle::g, g_comp_type, b_comp_type>>;
	template <gl::component_swizzle c>
	using comp_writer_type = std::conditional_t<c == gl::component_swizzle::r, r_comp_writer_type,
		std::conditional_t<c == gl::component_swizzle::g, g_comp_writer_type, b_comp_writer_type>>;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0_B, "bytes is zero");

	using data_t = std::uint8_t[static_cast<std::size_t>(bytes)];

	data_t data{};

public:
	block_3components() = default;
	block_3components(r_comp_type in_r, g_comp_type in_g, b_comp_type in_b) {
		r() = in_r;
		g() = in_g;
		b() = in_b;
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
	*	@brief 	Reads blue (3rd) component's value
	*/
	b_comp_type b() const { return _detail::block_component<type, b_bits, b_offset, true>(data); }

	/**
	*	@brief 	Provides read-write access to red (1st) component
	*/
	auto r() { return _detail::block_component<type, r_bits, r_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to green (2nd) component
	*/
	auto g() { return _detail::block_component<type, g_bits, g_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to blue (3rd) component
	*/
	auto b() { return _detail::block_component<type, b_bits, b_offset, false>(data); }

	template <int index>
	auto component() {
		static_assert(index >= 0 && index <= 2);
		if constexpr (index == r_index) return r();
		if constexpr (index == g_index) return g();
		if constexpr (index == b_index) return b();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 2);
		if constexpr (index == r_index) return r();
		if constexpr (index == g_index) return g();
		if constexpr (index == b_index) return b();
	}
};

}
}
