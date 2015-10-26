// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <gli/gli.hpp>

namespace StE {
namespace LLR {

template <gli::format type> struct surface_element_type {};

// unorm formats
template <> struct surface_element_type<gli::format::FORMAT_R8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_UNORM> { using type = std::uint8_t; };

template <> struct surface_element_type<gli::format::FORMAT_R16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_UNORM> { using type = std::uint16_t; };

// snorm formats
template <> struct surface_element_type<gli::format::FORMAT_R8_SNORM> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_SNORM> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_SNORM> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_SNORM> { using type = std::int8_t; };

template <> struct surface_element_type<gli::format::FORMAT_R16_SNORM> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_SNORM> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_SNORM> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_SNORM> { using type = std::int16_t; };

// Unsigned integer formats
template <> struct surface_element_type<gli::format::FORMAT_R8_UINT> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_UINT> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_UINT> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_UINT> { using type = std::uint8_t; };

template <> struct surface_element_type<gli::format::FORMAT_R16_UINT> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_UINT> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_UINT> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_UINT> { using type = std::uint16_t; };

template <> struct surface_element_type<gli::format::FORMAT_R32_UINT> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_UINT> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_UINT> { using type = std::uint32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_UINT> { using type = std::uint32_t; };

// Signed integer formats
template <> struct surface_element_type<gli::format::FORMAT_R8_SINT> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG8_SINT> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB8_SINT> { using type = std::int8_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA8_SINT> { using type = std::int8_t; };

template <> struct surface_element_type<gli::format::FORMAT_R16_SINT> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG16_SINT> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB16_SINT> { using type = std::int16_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA16_SINT> { using type = std::int16_t; };

template <> struct surface_element_type<gli::format::FORMAT_R32_SINT> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_SINT> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_SINT> { using type = std::int32_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_SINT> { using type = std::int32_t; };

// Floating formats
template <> struct surface_element_type<gli::format::FORMAT_R32_SFLOAT> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RG32_SFLOAT> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RGB32_SFLOAT> { using type = float; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA32_SFLOAT> { using type = float; };

// Swizzle formats
template <> struct surface_element_type<gli::format::FORMAT_BGR8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_BGRA8_UNORM> { using type = std::uint8_t; };

// Luminance Alpha formats
template <> struct surface_element_type<gli::format::FORMAT_L8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_A8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_LA8_UNORM> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_L16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_A16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_LA16_UNORM> { using type = std::uint16_t; };

// Depth formats
template <> struct surface_element_type<gli::format::FORMAT_D16_UNORM> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_D32_UFLOAT> { using type = float; };

}
}
