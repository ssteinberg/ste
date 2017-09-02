//	StE
// © Shlomi Steinberg 2015-2017

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

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::d) return d_index;
		static_assert(c == gl::component_swizzle::d);
		return -1;
	}

	static constexpr unsigned total_bits = d_bits + d_offset;
	static constexpr unsigned bytes = total_bits / 8;

	using d_comp_type = typename _detail::block_primary_type_selector<type, d_bits>::type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

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

	/**
	*	@brief Loads a block from memory and writes the decoded data to output buffer.
	*
	*	@param	data		Input buffer
	*	@param	d_output	Output buffer, must be able to hold sizeof(common_type) bytes
	 *	
	 *	@return	Element count written
	*/
	static std::size_t load_block(const std::uint8_t *data, common_type *d_output) {
		auto &block = *reinterpret_cast<const block_depth*>(data);
		*d_output = static_cast<common_type>(block.d());

		return 1;
	}

	/**
	*	@brief	Encodes from buffer to block.
	*
	*	@param	data			Input buffer
	*	@param	max_elements	Max elements to read
	*/
	template <typename src_type>
	void write_block(const src_type *data, std::size_t max_elements = 1) {
		d() = max_elements > 0 ? static_cast<d_comp_type>(*(data + 0)) : static_cast<d_comp_type>(0);
	}
};

}
}
