//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <format.hpp>
#include <image_view_swizzle.hpp>

#include <type_traits>
#include <half.hpp>

namespace ste {
namespace gl {

// format for scalar/vector
namespace _detail {
template<typename T, bool normalized = false>
struct _format_for_type {};
template<> struct _format_for_type<std::int8_t, false> { static constexpr format value = format::r8_sint; };
template<> struct _format_for_type<std::int16_t, false> { static constexpr format value = format::r16_sint; };
template<> struct _format_for_type<std::int32_t, false> { static constexpr format value = format::r32_sint; };
template<> struct _format_for_type<std::int64_t, false> { static constexpr format value = format::r64_sint; };
template<> struct _format_for_type<std::uint8_t, false> { static constexpr format value = format::r8_uint; };
template<> struct _format_for_type<std::uint16_t, false> { static constexpr format value = format::r16_uint; };
template<> struct _format_for_type<std::uint32_t, false> { static constexpr format value = format::r32_uint; };
template<> struct _format_for_type<std::uint64_t, false> { static constexpr format value = format::r64_uint; };
template<> struct _format_for_type<std::int8_t, true> { static constexpr format value = format::r8_snorm; };
template<> struct _format_for_type<std::int16_t, true> { static constexpr format value = format::r16_snorm; };
template<> struct _format_for_type<std::uint8_t, true> { static constexpr format value = format::r8_unorm; };
template<> struct _format_for_type<std::uint16_t, true> { static constexpr format value = format::r16_unorm; };
template<> struct _format_for_type<half_float::half, false> { static constexpr format value = format::r16_sfloat; };
template<> struct _format_for_type<glm::f32, false> { static constexpr format value = format::r32_sfloat; };
template<> struct _format_for_type<glm::f64, false> { static constexpr format value = format::r64_sfloat; };
template<> struct _format_for_type<glm::i8vec2, false> { static constexpr format value = format::r8g8_sint; };
template<> struct _format_for_type<glm::i16vec2, false> { static constexpr format value = format::r16g16_sint; };
template<> struct _format_for_type<glm::i32vec2, false> { static constexpr format value = format::r32g32_sint; };
template<> struct _format_for_type<glm::i64vec2, false> { static constexpr format value = format::r64g64_sint; };
template<> struct _format_for_type<glm::u8vec2, false> { static constexpr format value = format::r8g8_uint; };
template<> struct _format_for_type<glm::u16vec2, false> { static constexpr format value = format::r16g16_uint; };
template<> struct _format_for_type<glm::u32vec2, false> { static constexpr format value = format::r32g32_uint; };
template<> struct _format_for_type<glm::u64vec2, false> { static constexpr format value = format::r64g64_uint; };
template<> struct _format_for_type<glm::i8vec2, true> { static constexpr format value = format::r8g8_snorm; };
template<> struct _format_for_type<glm::i16vec2, true> { static constexpr format value = format::r16g16_snorm; };
template<> struct _format_for_type<glm::u8vec2, true> { static constexpr format value = format::r8g8_unorm; };
template<> struct _format_for_type<glm::u16vec2, true> { static constexpr format value = format::r16g16_unorm; };
template<> struct _format_for_type<glm::tvec2<half_float::half>, false> { static constexpr format value = format::r16g16_sfloat; };
template<> struct _format_for_type<glm::vec2, false> { static constexpr format value = format::r32g32_sfloat; };
template<> struct _format_for_type<glm::f64vec2, false> { static constexpr format value = format::r64g64_sfloat; };
template<> struct _format_for_type<glm::i8vec3, false> { static constexpr format value = format::r8g8b8_sint; };
template<> struct _format_for_type<glm::i16vec3, false> { static constexpr format value = format::r16g16b16_sint; };
template<> struct _format_for_type<glm::i32vec3, false> { static constexpr format value = format::r32g32b32_sint; };
template<> struct _format_for_type<glm::i64vec3, false> { static constexpr format value = format::r64g64b64_sint; };
template<> struct _format_for_type<glm::u8vec3, false> { static constexpr format value = format::r8g8b8_uint; };
template<> struct _format_for_type<glm::u16vec3, false> { static constexpr format value = format::r16g16b16_uint; };
template<> struct _format_for_type<glm::u32vec3, false> { static constexpr format value = format::r32g32b32_uint; };
template<> struct _format_for_type<glm::u64vec3, false> { static constexpr format value = format::r64g64b64_uint; };
template<> struct _format_for_type<glm::i8vec3, true> { static constexpr format value = format::r8g8b8_snorm; };
template<> struct _format_for_type<glm::i16vec3, true> { static constexpr format value = format::r16g16b16_snorm; };
template<> struct _format_for_type<glm::u8vec3, true> { static constexpr format value = format::r8g8b8_unorm; };
template<> struct _format_for_type<glm::u16vec3, true> { static constexpr format value = format::r16g16b16_unorm; };
template<> struct _format_for_type<glm::tvec3<half_float::half>, false> { static constexpr format value = format::r16g16b16_sfloat; };
template<> struct _format_for_type<glm::vec3, false> { static constexpr format value = format::r32g32b32_sfloat; };
template<> struct _format_for_type<glm::f64vec3, false> { static constexpr format value = format::r64g64b64_sfloat; };
template<> struct _format_for_type<glm::i8vec4, false> { static constexpr format value = format::r8g8b8a8_sint; };
template<> struct _format_for_type<glm::i16vec4, false> { static constexpr format value = format::r16g16b16a16_sint; };
template<> struct _format_for_type<glm::i32vec4, false> { static constexpr format value = format::r32g32b32a32_sint; };
template<> struct _format_for_type<glm::i64vec4, false> { static constexpr format value = format::r64g64b64a64_sint; };
template<> struct _format_for_type<glm::u8vec4, false> { static constexpr format value = format::r8g8b8a8_uint; };
template<> struct _format_for_type<glm::u16vec4, false> { static constexpr format value = format::r16g16b16a16_uint; };
template<> struct _format_for_type<glm::u32vec4, false> { static constexpr format value = format::r32g32b32a32_uint; };
template<> struct _format_for_type<glm::u64vec4, false> { static constexpr format value = format::r64g64b64a64_uint; };
template<> struct _format_for_type<glm::i8vec4, true> { static constexpr format value = format::r8g8b8a8_snorm; };
template<> struct _format_for_type<glm::i16vec4, true> { static constexpr format value = format::r16g16b16a16_snorm; };
template<> struct _format_for_type<glm::u8vec4, true> { static constexpr format value = format::r8g8b8a8_unorm; };
template<> struct _format_for_type<glm::u16vec4, true> { static constexpr format value = format::r16g16b16a16_unorm; };
template<> struct _format_for_type<glm::tvec4<half_float::half>, false> { static constexpr format value = format::r16g16b16a16_sfloat; };
template<> struct _format_for_type<glm::vec4, false> { static constexpr format value = format::r32g32b32a32_sfloat; };
template<> struct _format_for_type<glm::f64vec4, false> { static constexpr format value = format::r64g64b64a64_sfloat; };
}
template<typename T, bool normalized = false>
struct format_for_type {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto value = _detail::_format_for_type<TypeT, normalized>::value;
};
template<typename T, bool normalized = false>
static constexpr auto format_for_type_v = format_for_type<T, normalized>::value;


// Type traits for image format
template<format>
struct format_traits {};
template<> struct format_traits<format::r4g4_unorm_pack8> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
};
template<> struct format_traits<format::r4g4b4a4_unorm_pack16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::b4g4r4a4_unorm_pack16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
};
template<> struct format_traits<format::r5g6b5_unorm_pack16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::b5g6r5_unorm_pack16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::r5g5b5a1_unorm_pack16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::b5g5r5a1_unorm_pack16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
};
template<> struct format_traits<format::a1r5g5b5_unorm_pack16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::r8_unorm> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_UNORM_PACK8;
};
template<> struct format_traits<format::r8_snorm> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SNORM_PACK8;
};
template<> struct format_traits<format::r8_uscaled> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_USCALED_PACK8;
};
template<> struct format_traits<format::r8_sscaled> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SSCALED_PACK8;
};
template<> struct format_traits<format::r8_uint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_UINT_PACK8;
};
template<> struct format_traits<format::r8_sint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SINT_PACK8;
};
template<> struct format_traits<format::r8_srgb> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SRGB_PACK8;
};
template<> struct format_traits<format::r8g8_unorm> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_UNORM_PACK8;
};
template<> struct format_traits<format::r8g8_snorm> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SNORM_PACK8;
};
template<> struct format_traits<format::r8g8_uscaled> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_USCALED_PACK8;
};
template<> struct format_traits<format::r8g8_sscaled> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SSCALED_PACK8;
};
template<> struct format_traits<format::r8g8_uint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_UINT_PACK8;
};
template<> struct format_traits<format::r8g8_sint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SINT_PACK8;
};
template<> struct format_traits<format::r8g8_srgb> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SRGB_PACK8;
};
template<> struct format_traits<format::r8g8b8_unorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_UNORM_PACK8;
};
template<> struct format_traits<format::r8g8b8_snorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SNORM_PACK8;
};
template<> struct format_traits<format::r8g8b8_uscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_USCALED_PACK8;
};
template<> struct format_traits<format::r8g8b8_sscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SSCALED_PACK8;
};
template<> struct format_traits<format::r8g8b8_uint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_UINT_PACK8;
};
template<> struct format_traits<format::r8g8b8_sint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SINT_PACK8;
};
template<> struct format_traits<format::r8g8b8_srgb> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SRGB_PACK8;
};
template<> struct format_traits<format::b8g8r8_unorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_UNORM_PACK8;
};
template<> struct format_traits<format::b8g8r8_snorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SNORM_PACK8;
};
template<> struct format_traits<format::b8g8r8_uscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_USCALED_PACK8;
};
template<> struct format_traits<format::b8g8r8_sscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SSCALED_PACK8;
};
template<> struct format_traits<format::b8g8r8_uint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_UINT_PACK8;
};
template<> struct format_traits<format::b8g8r8_sint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SINT_PACK8;
};
template<> struct format_traits<format::b8g8r8_srgb> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SRGB_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_unorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_UNORM_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_snorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SNORM_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_uscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_USCALED_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_sscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SSCALED_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_uint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_UINT_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_sint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SINT_PACK8;
};
template<> struct format_traits<format::r8g8b8a8_srgb> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SRGB_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_unorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_UNORM_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_snorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SNORM_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_uscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_USCALED_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_sscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SSCALED_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_uint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_UINT_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_sint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SINT_PACK8;
};
template<> struct format_traits<format::b8g8r8a8_srgb> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SRGB_PACK8;
};
template<> struct format_traits<format::a8b8g8r8_unorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_snorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_uscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_sscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_uint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_sint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_srgb_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a2r10g10b10_unorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2r10g10b10_snorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2r10g10b10_uscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2r10g10b10_sscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2r10g10b10_uint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2r10g10b10_sint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::a2b10g10r10_unorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::a2b10g10r10_snorm_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::a2b10g10r10_uscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::a2b10g10r10_sscaled_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::a2b10g10r10_uint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::a2b10g10r10_sint_pack32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::r16_unorm> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_UNORM_PACK16;
};
template<> struct format_traits<format::r16_snorm> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SNORM_PACK16;
};
template<> struct format_traits<format::r16_uscaled> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_USCALED_PACK16;
};
template<> struct format_traits<format::r16_sscaled> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SSCALED_PACK16;
};
template<> struct format_traits<format::r16_uint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_UINT_PACK16;
};
template<> struct format_traits<format::r16_sint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SINT_PACK16;
};
template<> struct format_traits<format::r16_sfloat> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SFLOAT_PACK16;
};
template<> struct format_traits<format::r16g16_unorm> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_UNORM_PACK16;
};
template<> struct format_traits<format::r16g16_snorm> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SNORM_PACK16;
};
template<> struct format_traits<format::r16g16_uscaled> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_USCALED_PACK16;
};
template<> struct format_traits<format::r16g16_sscaled> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SSCALED_PACK16;
};
template<> struct format_traits<format::r16g16_uint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_UINT_PACK16;
};
template<> struct format_traits<format::r16g16_sint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SINT_PACK16;
};
template<> struct format_traits<format::r16g16_sfloat> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SFLOAT_PACK16;
};
template<> struct format_traits<format::r16g16b16_unorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_UNORM_PACK16;
};
template<> struct format_traits<format::r16g16b16_snorm> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SNORM_PACK16;
};
template<> struct format_traits<format::r16g16b16_uscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_USCALED_PACK16;
};
template<> struct format_traits<format::r16g16b16_sscaled> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SSCALED_PACK16;
};
template<> struct format_traits<format::r16g16b16_uint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_UINT_PACK16;
};
template<> struct format_traits<format::r16g16b16_sint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SINT_PACK16;
};
template<> struct format_traits<format::r16g16b16_sfloat> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SFLOAT_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_unorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_UNORM_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_snorm> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SNORM_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_uscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_USCALED_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_sscaled> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SSCALED_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_uint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_UINT_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_sint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SINT_PACK16;
};
template<> struct format_traits<format::r16g16b16a16_sfloat> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SFLOAT_PACK16;
};
template<> struct format_traits<format::r32_uint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_UINT_PACK32;
};
template<> struct format_traits<format::r32_sint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_SINT_PACK32;
};
template<> struct format_traits<format::r32_sfloat> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_SFLOAT_PACK32;
};
template<> struct format_traits<format::r32g32_uint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_UINT_PACK32;
};
template<> struct format_traits<format::r32g32_sint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_SINT_PACK32;
};
template<> struct format_traits<format::r32g32_sfloat> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_SFLOAT_PACK32;
};
template<> struct format_traits<format::r32g32b32_uint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_UINT_PACK32;
};
template<> struct format_traits<format::r32g32b32_sint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_SINT_PACK32;
};
template<> struct format_traits<format::r32g32b32_sfloat> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_SFLOAT_PACK32;
};
template<> struct format_traits<format::r32g32b32a32_uint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_UINT_PACK32;
};
template<> struct format_traits<format::r32g32b32a32_sint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_SINT_PACK32;
};
template<> struct format_traits<format::r32g32b32a32_sfloat> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_SFLOAT_PACK32;
};
template<> struct format_traits<format::r64_uint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_UINT_PACK64;
};
template<> struct format_traits<format::r64_sint> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_SINT_PACK64;
};
template<> struct format_traits<format::r64_sfloat> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_SFLOAT_PACK64;
};
template<> struct format_traits<format::r64g64_uint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_UINT_PACK64;
};
template<> struct format_traits<format::r64g64_sint> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_SINT_PACK64;
};
template<> struct format_traits<format::r64g64_sfloat> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_SFLOAT_PACK64;
};
template<> struct format_traits<format::r64g64b64_uint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_UINT_PACK64;
};
template<> struct format_traits<format::r64g64b64_sint> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_SINT_PACK64;
};
template<> struct format_traits<format::r64g64b64_sfloat> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_SFLOAT_PACK64;
};
template<> struct format_traits<format::r64g64b64a64_uint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_UINT_PACK64;
};
template<> struct format_traits<format::r64g64b64a64_sint> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_SINT_PACK64;
};
template<> struct format_traits<format::r64g64b64a64_sfloat> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_SFLOAT_PACK64;
};
template<> struct format_traits<format::b10g11r11_ufloat_pack32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::e5b9g9r9_ufloat_pack32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};
};
template<> struct format_traits<format::d16_unorm> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_D16_UNORM_PACK16;
};
template<> struct format_traits<format::x8_d24_unorm_pack32> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false;
};
template<> struct format_traits<format::d32_sfloat> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_D32_SFLOAT_PACK32;
};

}
}
