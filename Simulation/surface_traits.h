// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <gli/gli.hpp>

namespace StE {
namespace LLR {

template <gli::format type> struct surface_element_type {};
template <gli::format type> struct surface_is_signed : public std::false_type {};
template <gli::format type> struct surface_is_normalized : public std::false_type {};

// unorm formats
template <> struct surface_element_type<gli::format::FORMAT_R8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA8_UNORM_PACK8> : public std::true_type {};

template <> struct surface_element_type<gli::format::FORMAT_R16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA16_UNORM_PACK16> : public std::true_type {};

// snorm formats
template <> struct surface_element_type<gli::format::FORMAT_R8_SNORM_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_SNORM_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_SNORM_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_SNORM_PACK8> { using type = std::int8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB8_SNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA8_SNORM_PACK8> : public std::true_type {};

template <> struct surface_element_type<gli::format::FORMAT_R16_SNORM_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_SNORM_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_SNORM_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_SNORM_PACK16> { using type = std::int16_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB16_SNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA16_SNORM_PACK16> : public std::true_type {};

// Unsigned integer formats
template <> struct surface_element_type<gli::format::FORMAT_R8_UINT_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_UINT_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_UINT_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_UINT_PACK8> { using type = std::uint8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB8_UINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA8_UINT_PACK8> : public std::false_type {};

template <> struct surface_element_type<gli::format::FORMAT_R16_UINT_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_UINT_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_UINT_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_UINT_PACK16> { using type = std::uint16_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB16_UINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA16_UINT_PACK16> : public std::false_type {};

template <> struct surface_element_type<gli::format::FORMAT_R32_UINT_PACK32> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_UINT_PACK32> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_UINT_PACK32> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_UINT_PACK32> { using type = std::uint32_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB32_UINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA32_UINT_PACK32> : public std::false_type {};

// Signed integer formats
template <> struct surface_element_type<gli::format::FORMAT_R8_SINT_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_SINT_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_SINT_PACK8> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_SINT_PACK8> { using type = std::int8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R8_SINT_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG8_SINT_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB8_SINT_PACK8> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA8_SINT_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R8_SINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG8_SINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB8_SINT_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA8_SINT_PACK8> : public std::false_type {};

template <> struct surface_element_type<gli::format::FORMAT_R16_SINT_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_SINT_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_SINT_PACK16> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_SINT_PACK16> { using type = std::int16_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R16_SINT_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG16_SINT_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB16_SINT_PACK16> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA16_SINT_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R16_SINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG16_SINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB16_SINT_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA16_SINT_PACK16> : public std::false_type {};

template <> struct surface_element_type<gli::format::FORMAT_R32_SINT_PACK32> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_SINT_PACK32> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_SINT_PACK32> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_SINT_PACK32> { using type = std::int32_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R32_SINT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG32_SINT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB32_SINT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA32_SINT_PACK32> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R32_SINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG32_SINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB32_SINT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA32_SINT_PACK32> : public std::false_type {};

// Floating formats
template <> struct surface_element_type<gli::format::FORMAT_R32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_is_signed<gli::format::FORMAT_R32_SFLOAT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG32_SFLOAT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB32_SFLOAT_PACK32> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA32_SFLOAT_PACK32> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA32_SFLOAT_PACK32> : public std::false_type {};

// Swizzle formats
template <> struct surface_element_type<gli::format::FORMAT_BGR8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_BGRA8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_BGR8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_BGRA8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_BGR8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_BGRA8_UNORM_PACK8> : public std::true_type {};

// Luminance Alpha formats
template <> struct surface_element_type<gli::format::FORMAT_L8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_A8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_LA8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_L16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_A16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_LA16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_is_signed<gli::format::FORMAT_L8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_A8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_LA8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_L16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_A16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_LA16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_L8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_A8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_LA8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_L16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_A16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_LA16_UNORM_PACK16> : public std::true_type {};

// Depth formats
template <> struct surface_element_type<gli::format::FORMAT_D16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_D32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_is_signed<gli::format::FORMAT_D16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_D32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_D16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_D32_SFLOAT_PACK32> : public std::false_type {};

}
}
