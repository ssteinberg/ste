//	StE
// © Shlomi Steinberg 2015-2017

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

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		static_assert(c == gl::component_swizzle::r);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

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

	/**
	*	@brief Loads a block from memory and writes the decoded data to output buffer.
	*
	*	@param	data		Input buffer
	*	@param	r_output	Output buffer, must be able to hold sizeof(common_type) bytes
	 *	
	 *	@return	Element count written
	*/
	static std::size_t load_block(const std::uint8_t *data, common_type *r_output) {
		auto &block = *reinterpret_cast<const block_1components*>(data);
		*r_output = static_cast<common_type>(block.r());

		return 1;
	}

	template <typename src_type>
	void write_block(const src_type *data) {
		r() = static_cast<r_comp_type>(*data);
	}
};

}
}
