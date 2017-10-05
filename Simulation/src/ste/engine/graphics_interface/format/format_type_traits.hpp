//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <format.hpp>
#include <image_view_swizzle.hpp>
#include <image_type_traits.hpp>
#include <surface_block.hpp>

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
template<> struct _format_for_type<i8_norm, false> { static constexpr format value = format::r8_snorm; };
template<> struct _format_for_type<i16_norm, false> { static constexpr format value = format::r16_snorm; };
template<> struct _format_for_type<u8_norm, false> { static constexpr format value = format::r8_unorm; };
template<> struct _format_for_type<u16_norm, false> { static constexpr format value = format::r16_unorm; };
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
template<> struct _format_for_type<i8vec2_norm, false> { static constexpr format value = format::r8g8_snorm; };
template<> struct _format_for_type<i16vec2_norm, false> { static constexpr format value = format::r16g16_snorm; };
template<> struct _format_for_type<u8vec2_norm, false> { static constexpr format value = format::r8g8_unorm; };
template<> struct _format_for_type<u16vec2_norm, false> { static constexpr format value = format::r16g16_unorm; };
template<> struct _format_for_type<glm::tvec2<half_float::half>, false> { static constexpr format value = format::r16g16_sfloat; };
template<> struct _format_for_type<glm::vec2, false> { static constexpr format value = format::r32g32_sfloat; };
template<> struct _format_for_type<metre_vec2, false> { static constexpr format value = format::r32g32_sfloat; };
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
template<> struct _format_for_type<i8vec3_norm, false> { static constexpr format value = format::r8g8b8_snorm; };
template<> struct _format_for_type<i16vec3_norm, false> { static constexpr format value = format::r16g16b16_snorm; };
template<> struct _format_for_type<u8vec3_norm, false> { static constexpr format value = format::r8g8b8_unorm; };
template<> struct _format_for_type<u16vec3_norm, false> { static constexpr format value = format::r16g16b16_unorm; };
template<> struct _format_for_type<glm::tvec3<half_float::half>, false> { static constexpr format value = format::r16g16b16_sfloat; };
template<> struct _format_for_type<glm::vec3, false> { static constexpr format value = format::r32g32b32_sfloat; };
template<> struct _format_for_type<metre_vec3, false> { static constexpr format value = format::r32g32b32_sfloat; };
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
template<> struct _format_for_type<i8vec4_norm, false> { static constexpr format value = format::r8g8b8a8_snorm; };
template<> struct _format_for_type<i16vec4_norm, false> { static constexpr format value = format::r16g16b16a16_snorm; };
template<> struct _format_for_type<u8vec4_norm, false> { static constexpr format value = format::r8g8b8a8_unorm; };
template<> struct _format_for_type<u16vec4_norm, false> { static constexpr format value = format::r16g16b16a16_unorm; };
template<> struct _format_for_type<glm::tvec4<half_float::half>, false> { static constexpr format value = format::r16g16b16a16_sfloat; };
template<> struct _format_for_type<glm::vec4, false> { static constexpr format value = format::r32g32b32a32_sfloat; };
template<> struct _format_for_type<metre_vec4, false> { static constexpr format value = format::r32g32b32a32_sfloat; };
template<> struct _format_for_type<glm::f64vec4, false> { static constexpr format value = format::r64g64b64a64_sfloat; };
template<> struct _format_for_type<glm::tquat<half_float::half, glm::qualifier::lowp>, false> { static constexpr format value = format::r16g16b16a16_sfloat; };
template<> struct _format_for_type<glm::tquat<half_float::half, glm::qualifier::mediump>, false> { static constexpr format value = format::r16g16b16a16_sfloat; };
template<> struct _format_for_type<glm::tquat<half_float::half, glm::qualifier::highp>, false> { static constexpr format value = format::r16g16b16a16_sfloat; };
template<> struct _format_for_type<glm::quat, false> { static constexpr format value = format::r32g32b32a32_sfloat; };
template<> struct _format_for_type<glm::f64quat, false> { static constexpr format value = format::r64g64b64a64_sfloat; };
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
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };
	
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_unorm, 4, 4, components[0], components[1]>;
};
template<> struct format_traits<format::r4g4b4a4_unorm_pack16> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 4, 4, 4, 4, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::b4g4r4a4_unorm_pack16> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 4, 4, 4, 4, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::r5g6b5_unorm_pack16> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_unorm, 5, 6, 5, components[0], components[1], components[2]>;
};
template<> struct format_traits<format::b5g6r5_unorm_pack16> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_unorm, 5, 6, 5, components[0], components[1], components[2]>;
};
template<> struct format_traits<format::r5g5b5a1_unorm_pack16> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 5, 5, 5, 1, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::b5g5r5a1_unorm_pack16> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 5, 5, 5, 1, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a1r5g5b5_unorm_pack16> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
	
	using block_type = resource::block_4components<resource::block_type::block_unorm, 1, 5, 5, 5, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::r8_unorm> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_unorm, 8>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8_snorm> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_snorm, 8>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8_uscaled> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uscaled, 8>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8_sscaled> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sscaled, 8>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8_uint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uint, 8>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8_sint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sint, 8>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8_srgb> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 1;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_srgb, 8>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8_unorm> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_unorm, 8, 8, components[0], components[1]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8_snorm> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_snorm, 8, 8, components[0], components[1]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8_uscaled> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uscaled, 8, 8, components[0], components[1]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8_sscaled> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_sscaled, 8, 8, components[0], components[1]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8_uint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uint, 8, 8, components[0], components[1]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8_sint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_sint, 8, 8, components[0], components[1]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8_srgb> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_srgb, 8, 8, components[0], components[1]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8_unorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_unorm, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8_snorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_snorm, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8_uscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uscaled, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8_sscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sscaled, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8_uint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uint, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8_sint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sint, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8_srgb> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_srgb, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8_unorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_unorm, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8_snorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_snorm, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8_uscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_uscaled, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8_sscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_sscaled, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8_uint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_uint, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8_sint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_sint, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8_srgb> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 3;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_3components<resource::block_type::block_srgb, 8, 8, 8, components[0], components[1], components[2]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8a8_unorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8a8_snorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8a8_uscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8a8_sscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8a8_uint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::r8g8b8a8_sint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::r8g8b8a8_srgb> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_srgb, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8a8_unorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8a8_snorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8a8_uscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8a8_sscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8a8_uint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::b8g8r8a8_sint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::b8g8r8a8_srgb> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::b, component_swizzle::g, component_swizzle::r, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_srgb, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_unorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_snorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_uscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_sscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_uint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a8b8g8r8_sint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::int8_t;
};
template<> struct format_traits<format::a8b8g8r8_srgb_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_srgb, 8, 8, 8, 8, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint8_t;
};
template<> struct format_traits<format::a2r10g10b10_unorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2r10g10b10_snorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2r10g10b10_uscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2r10g10b10_sscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2r10g10b10_uint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2r10g10b10_sint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_unorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_snorm_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_uscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_sscaled_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_uint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::a2b10g10r10_sint_pack32> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::a, component_swizzle::b, component_swizzle::g, component_swizzle::r
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 2, 10, 10, 10, components[0], components[1], components[2], components[3]>;
};
template<> struct format_traits<format::r16_unorm> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_unorm, 16>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16_snorm> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_snorm, 16>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16_uscaled> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uscaled, 16>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16_sscaled> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sscaled, 16>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16_uint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uint, 16>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16_sint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sint, 16>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16_sfloat> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_fp, 16>;
	using element_type = half_float::half;
};
template<> struct format_traits<format::r16g16_unorm> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_unorm, 16, 16, components[0], components[1]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16_snorm> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_snorm, 16, 16, components[0], components[1]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16_uscaled> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uscaled, 16, 16, components[0], components[1]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16_sscaled> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_sscaled, 16, 16, components[0], components[1]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16_uint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uint, 16, 16, components[0], components[1]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16_sint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_sint, 16, 16, components[0], components[1]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16_sfloat> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_fp, 16, 16, components[0], components[1]>;
	using element_type = half_float::half;
};
template<> struct format_traits<format::r16g16b16_unorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_unorm, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16_snorm> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_snorm, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16b16_uscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uscaled, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16_sscaled> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sscaled, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16b16_uint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uint, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16_sint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sint, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16b16_sfloat> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 6;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_fp, 16, 16, 16, components[0], components[1], components[2]>;
	using element_type = half_float::half;
};
template<> struct format_traits<format::r16g16b16a16_unorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_unorm, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16a16_snorm> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_snorm, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16b16a16_uscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uscaled, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16a16_sscaled> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = true,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sscaled, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::int16_t;
};
template<> struct format_traits<format::r16g16b16a16_uint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16a16_sint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::r16g16b16a16_sfloat> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_fp, 16, 16, 16, 16, components[0], components[1], components[2], components[3]>;
	using element_type = half_float::half;
};
template<> struct format_traits<format::r32_uint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uint, 32>;
	using element_type = std::uint32_t;
};
template<> struct format_traits<format::r32_sint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sint, 32>;
	using element_type = std::int32_t;
};
template<> struct format_traits<format::r32_sfloat> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_fp, 32>;
	using element_type = float;
};
template<> struct format_traits<format::r32g32_uint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uint, 32, 32, components[0], components[1]>;
	using element_type = std::uint32_t;
};
template<> struct format_traits<format::r32g32_sint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uint, 32, 32, components[0], components[1]>;
	using element_type = std::int32_t;
};
template<> struct format_traits<format::r32g32_sfloat> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_fp, 32, 32, components[0], components[1]>;
	using element_type = float;
};
template<> struct format_traits<format::r32g32b32_uint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 12;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uint, 32, 32, 32, components[0], components[1], components[2]>;
	using element_type = std::uint32_t;
};
template<> struct format_traits<format::r32g32b32_sint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 12;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sint, 32, 32, 32, components[0], components[1], components[2]>;
	using element_type = std::int32_t;
};
template<> struct format_traits<format::r32g32b32_sfloat> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 12;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_fp, 32, 32, 32, components[0], components[1], components[2]>;
	using element_type = float;
};
template<> struct format_traits<format::r32g32b32a32_uint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 32, 32, 32, 32, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint32_t;
};
template<> struct format_traits<format::r32g32b32a32_sint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 32, 32, 32, 32, components[0], components[1], components[2], components[3]>;
	using element_type = std::int32_t;
};
template<> struct format_traits<format::r32g32b32a32_sfloat> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_fp, 32, 32, 32, 32, components[0], components[1], components[2], components[3]>;
	using element_type = float;
};
template<> struct format_traits<format::r64_uint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_uint, 64>;
	using element_type = std::uint64_t;
};
template<> struct format_traits<format::r64_sint> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_sint, 64>;
	using element_type = std::int64_t;
};
template<> struct format_traits<format::r64_sfloat> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};

	using block_type = resource::block_1components<resource::block_type::block_fp, 64>;
	using element_type = double;
};
template<> struct format_traits<format::r64g64_uint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_uint, 64, 64, components[0], components[1]>;
	using element_type = std::uint64_t;
};
template<> struct format_traits<format::r64g64_sint> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_sint, 64, 64, components[0], components[1]>;
	using element_type = std::int64_t;
};
template<> struct format_traits<format::r64g64_sfloat> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};

	using block_type = resource::block_2components<resource::block_type::block_fp, 64, 64, components[0], components[1]>;
	using element_type = double;
};
template<> struct format_traits<format::r64g64b64_uint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 24;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_uint, 64, 64, 64, components[0], components[1], components[2]>;
	using element_type = std::uint64_t;
};
template<> struct format_traits<format::r64g64b64_sint> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 24;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_sint, 64, 64, 64, components[0], components[1], components[2]>;
	using element_type = std::int64_t;
};
template<> struct format_traits<format::r64g64b64_sfloat> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 24;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};

	using block_type = resource::block_3components<resource::block_type::block_fp, 64, 64, 64, components[0], components[1], components[2]>;
	using element_type = double;
};
template<> struct format_traits<format::r64g64b64a64_uint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 32;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_uint, 64, 64, 64, 64, components[0], components[1], components[2], components[3]>;
	using element_type = std::uint64_t;
};
template<> struct format_traits<format::r64g64b64a64_sint> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 32;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_sint, 64, 64, 64, 64, components[0], components[1], components[2], components[3]>;
	using element_type = std::int64_t;
};
template<> struct format_traits<format::r64g64b64a64_sfloat> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 32;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};

	using block_type = resource::block_4components<resource::block_type::block_fp, 64, 64, 64, 64, components[0], components[1], components[2], components[3]>;
	using element_type = double;
};
template<> struct format_traits<format::d16_unorm> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 2;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::d
	};

	using block_type = resource::block_depth<resource::block_type::block_unorm, 16>;
	using element_type = std::uint16_t;
};
template<> struct format_traits<format::x8_d24_unorm_pack32> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::d
	};

	using block_type = resource::block_depth<resource::block_type::block_unorm, 24, 8>;
};
template<> struct format_traits<format::d32_sfloat> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 4;
	static constexpr image_extent_type_t<2> block_extent = { 1, 1 };

	static constexpr bool is_depth = true,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = false,
		is_scaled_integer = false,
		is_compressed = false;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::d
	};

	using block_type = resource::block_depth<resource::block_type::block_fp, 32>;
	using element_type = float;
};
template<> struct format_traits<format::bc1_rgb_unorm_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::bc1_rgb_srgb_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::bc1_rgba_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc1_rgba_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc2_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc2_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc3_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc3_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc4_unorm_block> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
};
template<> struct format_traits<format::bc4_snorm_block> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
};
template<> struct format_traits<format::bc5_unorm_block> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
};
template<> struct format_traits<format::bc5_snorm_block> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g
	};
};
template<> struct format_traits<format::bc6h_ufloat_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::bc6h_sfloat_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::bc7_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::bc7_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::etc2_r8g8b8_unorm_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::etc2_r8g8b8_srgb_block> {
	static constexpr std::uint8_t elements = 3;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b
	};
};
template<> struct format_traits<format::etc2_r8g8b8a1_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::etc2_r8g8b8a1_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::etc2_r8g8b8a8_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::etc2_r8g8b8a8_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::eac_r11_unorm_block> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
};
template<> struct format_traits<format::eac_r11_snorm_block> {
	static constexpr std::uint8_t elements = 1;
	static constexpr std::uint8_t block_bytes = 8;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r
	};
};
template<> struct format_traits<format::eac_r11g11_unorm_block> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r,component_swizzle::g
	};
};
template<> struct format_traits<format::eac_r11g11_snorm_block> {
	static constexpr std::uint8_t elements = 2;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r,component_swizzle::g
	};
};
template<> struct format_traits<format::astc_4x4_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_4x4_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 4, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_5x4_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 5, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::d
	};
};
template<> struct format_traits<format::astc_5x4_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 5, 4 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_5x5_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 5, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_5x5_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 5, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_6x5_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 6, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_6x5_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 6, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_6x6_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 6, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_6x6_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 6, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x5_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x5_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x6_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x6_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x8_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 8 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_8x8_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 8, 8 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x5_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x5_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 5 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x6_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x6_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 6 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x8_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 8 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x8_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 8 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x10_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 10 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_10x10_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 10, 10 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_12x10_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 12, 10 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_12x10_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 12, 10 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_12x12_unorm_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 12, 12 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = false,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};
template<> struct format_traits<format::astc_12x12_srgb_block> {
	static constexpr std::uint8_t elements = 4;
	static constexpr std::uint8_t block_bytes = 16;
	static constexpr image_extent_type_t<2> block_extent = { 12, 12 };

	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false,
		is_srgb = true,
		is_normalized_integer = true,
		is_scaled_integer = false,
		is_compressed = true;

	static constexpr component_swizzle components[elements] = {
		component_swizzle::r, component_swizzle::g, component_swizzle::b, component_swizzle::a
	};
};

}
}
