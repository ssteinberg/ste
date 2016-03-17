// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include <gli/gli.hpp>

namespace StE {
namespace LLR {

template <gli::format type> struct surface_element_type {};
template <gli::format type> struct surface_is_signed : public std::false_type {};
template <gli::format type> struct surface_is_normalized : public std::false_type {};
template <gli::format type> struct surface_stride{};
template <gli::format type> struct surface_elements{};

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
template <> struct surface_stride<gli::format::FORMAT_R8_UNORM_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_stride<gli::format::FORMAT_RG8_UNORM_PACK8> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RGB8_UNORM_PACK8> { static constexpr std::size_t value = 24; };
template <> struct surface_stride<gli::format::FORMAT_RGBA8_UNORM_PACK8> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_R8_UNORM_PACK8> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG8_UNORM_PACK8> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB8_UNORM_PACK8> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA8_UNORM_PACK8> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R16_UNORM_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RG16_UNORM_PACK16> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RGB16_UNORM_PACK16> { static constexpr std::size_t value = 48; };
template <> struct surface_stride<gli::format::FORMAT_RGBA16_UNORM_PACK16> { static constexpr std::size_t value = 64; };
template <> struct surface_elements<gli::format::FORMAT_R16_UNORM_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG16_UNORM_PACK16> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB16_UNORM_PACK16> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA16_UNORM_PACK16> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R8_SNORM_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_stride<gli::format::FORMAT_RG8_SNORM_PACK8> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RGB8_SNORM_PACK8> { static constexpr std::size_t value = 24; };
template <> struct surface_stride<gli::format::FORMAT_RGBA8_SNORM_PACK8> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_R8_SNORM_PACK8> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG8_SNORM_PACK8> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB8_SNORM_PACK8> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA8_SNORM_PACK8> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R16_SNORM_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RG16_SNORM_PACK16> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RGB16_SNORM_PACK16> { static constexpr std::size_t value = 48; };
template <> struct surface_stride<gli::format::FORMAT_RGBA16_SNORM_PACK16> { static constexpr std::size_t value = 64; };
template <> struct surface_elements<gli::format::FORMAT_R16_SNORM_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG16_SNORM_PACK16> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB16_SNORM_PACK16> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA16_SNORM_PACK16> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R8_UINT_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_stride<gli::format::FORMAT_RG8_UINT_PACK8> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RGB8_UINT_PACK8> { static constexpr std::size_t value = 24; };
template <> struct surface_stride<gli::format::FORMAT_RGBA8_UINT_PACK8> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_R8_UINT_PACK8> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG8_UINT_PACK8> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB8_UINT_PACK8> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA8_UINT_PACK8> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R16_UINT_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RG16_UINT_PACK16> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RGB16_UINT_PACK16> { static constexpr std::size_t value = 48; };
template <> struct surface_stride<gli::format::FORMAT_RGBA16_UINT_PACK16> { static constexpr std::size_t value = 64; };
template <> struct surface_elements<gli::format::FORMAT_R16_UINT_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG16_UINT_PACK16> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB16_UINT_PACK16> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA16_UINT_PACK16> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R32_UINT_PACK32> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RG32_UINT_PACK32> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RGB32_UINT_PACK32> { static constexpr std::size_t value = 96; };
template <> struct surface_stride<gli::format::FORMAT_RGBA32_UINT_PACK32> { static constexpr std::size_t value = 128; };
template <> struct surface_elements<gli::format::FORMAT_R32_UINT_PACK32> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG32_UINT_PACK32> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB32_UINT_PACK32> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA32_UINT_PACK32> { static constexpr std::size_t value = 4; };

template <> struct surface_element_type<gli::format::FORMAT_R64_UINT_PACK64> { using type = std::uint64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG64_UINT_PACK64> { using type = std::uint64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB64_UINT_PACK64> { using type = std::uint64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA64_UINT_PACK64> { using type = std::uint64_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB64_UINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA64_UINT_PACK64> : public std::false_type {};
template <> struct surface_stride<gli::format::FORMAT_R64_UINT_PACK64> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RG64_UINT_PACK64> { static constexpr std::size_t value = 128; };
template <> struct surface_stride<gli::format::FORMAT_RGB64_UINT_PACK64> { static constexpr std::size_t value = 192; };
template <> struct surface_stride<gli::format::FORMAT_RGBA64_UINT_PACK64> { static constexpr std::size_t value = 256; };
template <> struct surface_elements<gli::format::FORMAT_R64_UINT_PACK64> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG64_UINT_PACK64> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB64_UINT_PACK64> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA64_UINT_PACK64> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R8_SINT_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_stride<gli::format::FORMAT_RG8_SINT_PACK8> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RGB8_SINT_PACK8> { static constexpr std::size_t value = 24; };
template <> struct surface_stride<gli::format::FORMAT_RGBA8_SINT_PACK8> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_R8_SINT_PACK8> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG8_SINT_PACK8> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB8_SINT_PACK8> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA8_SINT_PACK8> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R16_SINT_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_RG16_SINT_PACK16> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RGB16_SINT_PACK16> { static constexpr std::size_t value = 48; };
template <> struct surface_stride<gli::format::FORMAT_RGBA16_SINT_PACK16> { static constexpr std::size_t value = 64; };
template <> struct surface_elements<gli::format::FORMAT_R16_SINT_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG16_SINT_PACK16> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB16_SINT_PACK16> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA16_SINT_PACK16> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R32_SINT_PACK32> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RG32_SINT_PACK32> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RGB32_SINT_PACK32> { static constexpr std::size_t value = 96; };
template <> struct surface_stride<gli::format::FORMAT_RGBA32_SINT_PACK32> { static constexpr std::size_t value = 128; };
template <> struct surface_elements<gli::format::FORMAT_R32_SINT_PACK32> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG32_SINT_PACK32> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB32_SINT_PACK32> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA32_SINT_PACK32> { static constexpr std::size_t value = 4; };

template <> struct surface_element_type<gli::format::FORMAT_R64_SINT_PACK64> { using type = std::int64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RG64_SINT_PACK64> { using type = std::int64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGB64_SINT_PACK64> { using type = std::int64_t; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA64_SINT_PACK64> { using type = std::int64_t; };
template <> struct surface_is_signed<gli::format::FORMAT_R64_SINT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG64_SINT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB64_SINT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA64_SINT_PACK64> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R64_SINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG64_SINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB64_SINT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA64_SINT_PACK64> : public std::false_type {};
template <> struct surface_stride<gli::format::FORMAT_R64_SINT_PACK64> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RG64_SINT_PACK64> { static constexpr std::size_t value = 128; };
template <> struct surface_stride<gli::format::FORMAT_RGB64_SINT_PACK64> { static constexpr std::size_t value = 192; };
template <> struct surface_stride<gli::format::FORMAT_RGBA64_SINT_PACK64> { static constexpr std::size_t value = 256; };
template <> struct surface_elements<gli::format::FORMAT_R64_SINT_PACK64> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG64_SINT_PACK64> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB64_SINT_PACK64> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA64_SINT_PACK64> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_R32_SFLOAT_PACK32> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_RG32_SFLOAT_PACK32> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RGB32_SFLOAT_PACK32> { static constexpr std::size_t value = 92; };
template <> struct surface_stride<gli::format::FORMAT_RGBA32_SFLOAT_PACK32> { static constexpr std::size_t value = 128; };
template <> struct surface_elements<gli::format::FORMAT_R32_SFLOAT_PACK32> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG32_SFLOAT_PACK32> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB32_SFLOAT_PACK32> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA32_SFLOAT_PACK32> { static constexpr std::size_t value = 4; };

template <> struct surface_element_type<gli::format::FORMAT_R64_SFLOAT_PACK64> { using type = double; };
template <> struct surface_element_type<gli::format::FORMAT_RG64_SFLOAT_PACK64> { using type = double; };
template <> struct surface_element_type<gli::format::FORMAT_RGB64_SFLOAT_PACK64> { using type = double; };
template <> struct surface_element_type<gli::format::FORMAT_RGBA64_SFLOAT_PACK64> { using type = double; };
template <> struct surface_is_signed<gli::format::FORMAT_R64_SFLOAT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RG64_SFLOAT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGB64_SFLOAT_PACK64> : public std::true_type {};
template <> struct surface_is_signed<gli::format::FORMAT_RGBA64_SFLOAT_PACK64> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_R64_SFLOAT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RG64_SFLOAT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGB64_SFLOAT_PACK64> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_RGBA64_SFLOAT_PACK64> : public std::false_type {};
template <> struct surface_stride<gli::format::FORMAT_R64_SFLOAT_PACK64> { static constexpr std::size_t value = 64; };
template <> struct surface_stride<gli::format::FORMAT_RG64_SFLOAT_PACK64> { static constexpr std::size_t value = 128; };
template <> struct surface_stride<gli::format::FORMAT_RGB64_SFLOAT_PACK64> { static constexpr std::size_t value = 192; };
template <> struct surface_stride<gli::format::FORMAT_RGBA64_SFLOAT_PACK64> { static constexpr std::size_t value = 256; };
template <> struct surface_elements<gli::format::FORMAT_R64_SFLOAT_PACK64> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_RG64_SFLOAT_PACK64> { static constexpr std::size_t value = 2; };
template <> struct surface_elements<gli::format::FORMAT_RGB64_SFLOAT_PACK64> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_RGBA64_SFLOAT_PACK64> { static constexpr std::size_t value = 4; };

// Swizzle formats
template <> struct surface_element_type<gli::format::FORMAT_BGR8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_element_type<gli::format::FORMAT_BGRA8_UNORM_PACK8> { using type = std::uint8_t; };
template <> struct surface_is_signed<gli::format::FORMAT_BGR8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_BGRA8_UNORM_PACK8> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_BGR8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_BGRA8_UNORM_PACK8> : public std::true_type {};
template <> struct surface_stride<gli::format::FORMAT_BGR8_UNORM_PACK8> { static constexpr std::size_t value = 24; };
template <> struct surface_stride<gli::format::FORMAT_BGRA8_UNORM_PACK8> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_BGR8_UNORM_PACK8> { static constexpr std::size_t value = 3; };
template <> struct surface_elements<gli::format::FORMAT_BGRA8_UNORM_PACK8> { static constexpr std::size_t value = 4; };

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
template <> struct surface_stride<gli::format::FORMAT_A16_UNORM_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_LA16_UNORM_PACK16> { static constexpr std::size_t value = 32; };
template <> struct surface_stride<gli::format::FORMAT_L8_UNORM_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_stride<gli::format::FORMAT_A8_UNORM_PACK8> { static constexpr std::size_t value = 8; };
template <> struct surface_elements<gli::format::FORMAT_LA8_UNORM_PACK8> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_L16_UNORM_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_A16_UNORM_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_LA16_UNORM_PACK16> { static constexpr std::size_t value = 2; };

// Depth formats
template <> struct surface_element_type<gli::format::FORMAT_D16_UNORM_PACK16> { using type = std::uint16_t; };
template <> struct surface_element_type<gli::format::FORMAT_D32_SFLOAT_PACK32> { using type = float; };
template <> struct surface_is_signed<gli::format::FORMAT_D16_UNORM_PACK16> : public std::false_type {};
template <> struct surface_is_signed<gli::format::FORMAT_D32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_D16_UNORM_PACK16> : public std::true_type {};
template <> struct surface_is_normalized<gli::format::FORMAT_D32_SFLOAT_PACK32> : public std::false_type {};
template <> struct surface_stride<gli::format::FORMAT_D16_UNORM_PACK16> { static constexpr std::size_t value = 16; };
template <> struct surface_stride<gli::format::FORMAT_D32_SFLOAT_PACK32> { static constexpr std::size_t value = 32; };
template <> struct surface_elements<gli::format::FORMAT_D16_UNORM_PACK16> { static constexpr std::size_t value = 1; };
template <> struct surface_elements<gli::format::FORMAT_D32_SFLOAT_PACK32> { static constexpr std::size_t value = 1; };

}
}
