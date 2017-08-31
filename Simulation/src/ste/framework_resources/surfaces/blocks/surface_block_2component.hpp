//	StE
// © Shlomi Steinberg 2015-2017

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

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

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
		if constexpr (index == 0) return r();
		if constexpr (index == 1) return g();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 1);
		if constexpr (index == 0) return r();
		if constexpr (index == 1) return g();
	}

	/**
	*	@brief Loads a block from memory and writes the decoded data, in RG swizzling, to output buffer.
	*
	*	@param	data		Input buffer
	*	@param	rg_output	Output buffer, must be able to hold sizeof(common_type)*2 bytes
	 *	
	 *	@return	Element count written
	*/
	static std::size_t load_block(const std::uint8_t *data, common_type *rg_output) {
		auto &block = *reinterpret_cast<const block_2components*>(data);
		*(rg_output + 0) = static_cast<common_type>(block.r());
		*(rg_output + 1) = static_cast<common_type>(block.g());

		return 2;
	}

	template <typename src_type>
	void write_block(const src_type *data) {
		r() = static_cast<r_comp_type>(*(data + 0));
		g() = static_cast<g_comp_type>(*(data + 1));
	}
};

}
}
