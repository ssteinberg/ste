//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <srgb.hpp>
#include <half.hpp>

#include <image_view_swizzle.hpp>
#include <type_traits>

namespace ste {
namespace resource {

/**
 *	@brief	Describes the block's data properties
 */
enum class block_type {
	block_unorm,
	block_snorm,
	block_uscaled,
	block_sscaled,
	block_srgb,
	block_uint,
	block_sint,
	block_fp,
};

/**
*	@brief	Names a blocks 'common type'. Common types are used for transfer and conversion between opaque surfaces and surfaces.
*/
enum class block_common_type {
	fp32,
	fp64,
	int32,
	int64,
	uint32,
	uint64
};

namespace _detail {

template <block_type blocktype, int element_bits>
struct block_common_type_selector {
	static_assert(element_bits <= 64);
	static_assert(element_bits > 0);

	static constexpr bool is_signed = blocktype == block_type::block_snorm || blocktype == block_type::block_sscaled || blocktype == block_type::block_sint;
	static constexpr bool uses_fp = blocktype != block_type::block_uint && blocktype != block_type::block_sint;

	using fp_t = std::conditional_t<element_bits <= 32, float, double>;
	using integer_t = std::conditional_t<element_bits <= 32, std::int32_t, std::int64_t>;
	using type = std::conditional_t<uses_fp, fp_t, std::conditional_t<is_signed, integer_t, std::make_unsigned_t<integer_t>>>;

	static constexpr block_common_type common_type_name = std::is_same_v<type, float> ? block_common_type::fp32 :
		(std::is_same_v<type, double> ? block_common_type::fp64 :
		(std::is_same_v<type, std::int32_t> ? block_common_type::int32 :
		 (std::is_same_v<type, std::int64_t> ? block_common_type::int64 :
		 (std::is_same_v<type, std::uint32_t> ? block_common_type::uint32 : block_common_type::uint64))));
	static_assert(common_type_name != block_common_type::uint64 || std::is_same_v<type, std::uint64_t>);
};

template <block_type blocktype, int element_bits>
struct block_primary_type_selector {
	static_assert(element_bits <= 64);
	static_assert(element_bits > 0);

	static constexpr bool is_signed = blocktype == block_type::block_snorm || blocktype == block_type::block_sscaled || blocktype == block_type::block_sint;

	using fp_t = std::conditional_t<element_bits <= 32, float, double>;
	using integer_t = std::conditional_t<
		element_bits == 1, bool, std::conditional_t<
		element_bits <= 8, std::int8_t, std::conditional_t<
		element_bits <= 16, std::int16_t, std::conditional_t<
		element_bits <= 32, std::int32_t,
		std::int64_t>>>>;
	using uinteger_t = std::conditional_t<
		element_bits == 1, bool, std::conditional_t<
		element_bits <= 8, std::uint8_t, std::conditional_t<
		element_bits <= 16, std::uint16_t, std::conditional_t<
		element_bits <= 32, std::uint32_t,
		std::uint64_t>>>>;

	static constexpr bool uses_fp = blocktype != block_type::block_uint && blocktype != block_type::block_sint;

	using read_type = std::conditional_t<element_bits <= 32, std::uint32_t, std::uint64_t>;
	using type = std::conditional_t<uses_fp, fp_t, std::conditional_t<is_signed, integer_t, uinteger_t>>;
	using block_writer_type = type;
};
template <int element_bits>
struct block_primary_type_selector<block_type::block_fp, element_bits> {
	static_assert(element_bits <= 64);
	static_assert(element_bits == 16 || element_bits == 32 || element_bits == 64);	// fp16, fp32, fp64 only

	static constexpr bool is_signed = false;
	using read_type = std::conditional_t<element_bits <= 32, std::uint32_t, std::uint64_t>;
	using type = std::conditional_t<element_bits == 32, float, std::conditional_t<element_bits == 64, double, half_float::half>>;

	// half doesn't like conversions from anything but float, so for block writer functions avoid forcing half.
	using block_writer_type = std::conditional_t<element_bits == 64, double, float>;
};

/**
 *	@brief	A single component (element) of a block. Provides encode/decode facilities as well as set and get value.
 */
template <block_type type, unsigned size_bits, unsigned offset_bits, bool is_const>
class block_component {
	using T = typename block_primary_type_selector<type, size_bits>::type;
	using read_type = typename block_primary_type_selector<type, size_bits>::read_type;

	using ptr_t = std::conditional_t<is_const, const std::uint8_t*, std::uint8_t*>;

	ptr_t b;

	static constexpr auto mask() {
		static_assert(size_bits <= 64);
		if constexpr (size_bits == 64)
			return ~(0ull);
		if constexpr (size_bits < 64)
			return (1ull << size_bits) - 1ull;
		return 0ull;
	}

	read_type read() const {
		return (*reinterpret_cast<const read_type*>(b + offset_bits / 8) >> (offset_bits % 8)) & mask();
	}
	template <bool checker = is_const, typename = typename std::enable_if_t<!checker>>
	void write(read_type r) {
		auto m = mask() << (offset_bits % 8);
		auto &val = *reinterpret_cast<read_type*>(b + offset_bits / 8);

		// Zero out bits
		val &= ~m;
		// Write new bits
		val += (r << (offset_bits % 8)) & m;
	}

	static T convert_read_to_public(read_type data) {
		if constexpr (type == block_type::block_unorm)
			return static_cast<T>(data) / static_cast<T>(mask());
		if constexpr (type == block_type::block_snorm)
			return static_cast<T>(data) / static_cast<T>((1ull << (size_bits - 1)) - (data > 0 ? 1ull : 0ul));
		if constexpr (type == block_type::block_uscaled || type == block_type::block_sscaled)
			return static_cast<T>(data);
		if constexpr (type == block_type::block_srgb)
			return graphics::sRGB_to_linear<T>(static_cast<T>(data) / static_cast<T>(mask()));
		if constexpr (type == block_type::block_uint)
			return static_cast<T>(data);
		if constexpr (type == block_type::block_sint)
			return static_cast<T>(data);
		if constexpr (type == block_type::block_fp) {
			std::uint64_t temp = 0;
			*reinterpret_cast<read_type*>(&temp) = data;
			return *reinterpret_cast<const T*>(&temp);
		}
	}
	static read_type convert_public_to_read(T data) {
		if constexpr (type == block_type::block_unorm)
			return static_cast<read_type>(glm::round(glm::clamp<T>(data, 0, 1) * static_cast<T>(mask())));
		if constexpr (type == block_type::block_srgb) {
			data = graphics::linear_to_sRGB<T>(data);
			return static_cast<read_type>(glm::round(glm::clamp<T>(data, 0, 1) * static_cast<T>(mask())));
		}
		if constexpr (type == block_type::block_snorm || type == block_type::block_unorm) {
			auto max = (1ull << (size_bits - 1)) - (data > 0 ? 1ull : 0ul);
			return static_cast<read_type>(glm::round(glm::clamp<T>(data, -1, 1) * static_cast<T>(max)));
		}
		if constexpr (type == block_type::block_sscaled || type == block_type::block_uscaled)
			return static_cast<read_type>(glm::round(data));
		if constexpr (type == block_type::block_uint)
			return static_cast<read_type>(data);
		if constexpr (type == block_type::block_sint)
			return static_cast<read_type>(data);
		if constexpr (type == block_type::block_fp) {
			std::uint64_t temp = 0;
			*reinterpret_cast<T*>(&temp) = data;
			return *reinterpret_cast<const read_type*>(&temp);
		}
	}

public:
	block_component() = default;
	block_component(ptr_t b) : b(b) {}

	/**
	 *	@brief	Assign new value to the component.
	 */
	template <bool checker = is_const, typename = typename std::enable_if_t<!checker>>
	block_component &operator=(const T &input) {
		write(convert_public_to_read(input));
		return *this;
	}

	/**
	*	@brief	Conversion operator that reads component's value
	*/
	operator T() const {
		return convert_read_to_public(read());
	}
};

}

}
}

#include <surface_block_1component.hpp>
#include <surface_block_2component.hpp>
#include <surface_block_3component.hpp>
#include <surface_block_4component.hpp>
#include <surface_block_depth.hpp>
