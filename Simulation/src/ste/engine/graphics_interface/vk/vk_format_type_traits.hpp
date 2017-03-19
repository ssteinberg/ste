//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <type_traits>

#include <half.hpp>

namespace StE {
namespace GL {

// vk format for scalar/vector
namespace _detail {
template<typename T, bool normalized = false>
struct _vk_format_for_type {};
template<> struct _vk_format_for_type<std::int8_t, false> { static constexpr VkFormat format = VK_FORMAT_R8_SINT; };
template<> struct _vk_format_for_type<std::int16_t, false> { static constexpr VkFormat format = VK_FORMAT_R16_SINT; };
template<> struct _vk_format_for_type<std::int32_t, false> { static constexpr VkFormat format = VK_FORMAT_R32_SINT; };
template<> struct _vk_format_for_type<std::int64_t, false> { static constexpr VkFormat format = VK_FORMAT_R64_SINT; };
template<> struct _vk_format_for_type<std::uint8_t, false> { static constexpr VkFormat format = VK_FORMAT_R8_UINT; };
template<> struct _vk_format_for_type<std::uint16_t, false> { static constexpr VkFormat format = VK_FORMAT_R16_UINT; };
template<> struct _vk_format_for_type<std::uint32_t, false> { static constexpr VkFormat format = VK_FORMAT_R32_UINT; };
template<> struct _vk_format_for_type<std::uint64_t, false> { static constexpr VkFormat format = VK_FORMAT_R64_UINT; };
template<> struct _vk_format_for_type<std::int8_t, true> { static constexpr VkFormat format = VK_FORMAT_R8_SNORM; };
template<> struct _vk_format_for_type<std::int16_t, true> { static constexpr VkFormat format = VK_FORMAT_R16_SNORM; };
template<> struct _vk_format_for_type<std::uint8_t, true> { static constexpr VkFormat format = VK_FORMAT_R8_UNORM; };
template<> struct _vk_format_for_type<std::uint16_t, true> { static constexpr VkFormat format = VK_FORMAT_R16_UNORM; };
template<> struct _vk_format_for_type<half_float::half, false> { static constexpr VkFormat format = VK_FORMAT_R16_SFLOAT; };
template<> struct _vk_format_for_type<glm::f32, false> { static constexpr VkFormat format = VK_FORMAT_R32_SFLOAT; };
template<> struct _vk_format_for_type<glm::f64, false> { static constexpr VkFormat format = VK_FORMAT_R64_SFLOAT; };
template<> struct _vk_format_for_type<glm::i8vec2, false> { static constexpr VkFormat format = VK_FORMAT_R8G8_SINT; };
template<> struct _vk_format_for_type<glm::i16vec2, false> { static constexpr VkFormat format = VK_FORMAT_R16G16_SINT; };
template<> struct _vk_format_for_type<glm::i32vec2, false> { static constexpr VkFormat format = VK_FORMAT_R32G32_SINT; };
template<> struct _vk_format_for_type<glm::i64vec2, false> { static constexpr VkFormat format = VK_FORMAT_R64G64_SINT; };
template<> struct _vk_format_for_type<glm::u8vec2, false> { static constexpr VkFormat format = VK_FORMAT_R8G8_UINT; };
template<> struct _vk_format_for_type<glm::u16vec2, false> { static constexpr VkFormat format = VK_FORMAT_R16G16_UINT; };
template<> struct _vk_format_for_type<glm::u32vec2, false> { static constexpr VkFormat format = VK_FORMAT_R32G32_UINT; };
template<> struct _vk_format_for_type<glm::u64vec2, false> { static constexpr VkFormat format = VK_FORMAT_R64G64_UINT; };
template<> struct _vk_format_for_type<glm::i8vec2, true> { static constexpr VkFormat format = VK_FORMAT_R8G8_SNORM; };
template<> struct _vk_format_for_type<glm::i16vec2, true> { static constexpr VkFormat format = VK_FORMAT_R16G16_SNORM; };
template<> struct _vk_format_for_type<glm::u8vec2, true> { static constexpr VkFormat format = VK_FORMAT_R8G8_UNORM; };
template<> struct _vk_format_for_type<glm::u16vec2, true> { static constexpr VkFormat format = VK_FORMAT_R16G16_UNORM; };
template<> struct _vk_format_for_type<glm::tvec2<half_float::half>, false> { static constexpr VkFormat format = VK_FORMAT_R16G16_SFLOAT; };
template<> struct _vk_format_for_type<glm::vec2, false> { static constexpr VkFormat format = VK_FORMAT_R32G32_SFLOAT; };
template<> struct _vk_format_for_type<glm::f64vec2, false> { static constexpr VkFormat format = VK_FORMAT_R64G64_SFLOAT; };
template<> struct _vk_format_for_type<glm::i8vec3, false> { static constexpr VkFormat format = VK_FORMAT_R8G8B8_SINT; };
template<> struct _vk_format_for_type<glm::i16vec3, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16_SINT; };
template<> struct _vk_format_for_type<glm::i32vec3, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32_SINT; };
template<> struct _vk_format_for_type<glm::i64vec3, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64_SINT; };
template<> struct _vk_format_for_type<glm::u8vec3, false> { static constexpr VkFormat format = VK_FORMAT_R8G8B8_UINT; };
template<> struct _vk_format_for_type<glm::u16vec3, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16_UINT; };
template<> struct _vk_format_for_type<glm::u32vec3, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32_UINT; };
template<> struct _vk_format_for_type<glm::u64vec3, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64_UINT; };
template<> struct _vk_format_for_type<glm::i8vec3, true> { static constexpr VkFormat format = VK_FORMAT_R8G8B8_SNORM; };
template<> struct _vk_format_for_type<glm::i16vec3, true> { static constexpr VkFormat format = VK_FORMAT_R16G16B16_SNORM; };
template<> struct _vk_format_for_type<glm::u8vec3, true> { static constexpr VkFormat format = VK_FORMAT_R8G8B8_UNORM; };
template<> struct _vk_format_for_type<glm::u16vec3, true> { static constexpr VkFormat format = VK_FORMAT_R16G16B16_UNORM; };
template<> struct _vk_format_for_type<glm::tvec3<half_float::half>, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16_SFLOAT; };
template<> struct _vk_format_for_type<glm::vec3, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32_SFLOAT; };
template<> struct _vk_format_for_type<glm::f64vec3, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64_SFLOAT; };
template<> struct _vk_format_for_type<glm::i8vec4, false> { static constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SINT; };
template<> struct _vk_format_for_type<glm::i16vec4, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SINT; };
template<> struct _vk_format_for_type<glm::i32vec4, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_SINT; };
template<> struct _vk_format_for_type<glm::i64vec4, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64A64_SINT; };
template<> struct _vk_format_for_type<glm::u8vec4, false> { static constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UINT; };
template<> struct _vk_format_for_type<glm::u16vec4, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16A16_UINT; };
template<> struct _vk_format_for_type<glm::u32vec4, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_UINT; };
template<> struct _vk_format_for_type<glm::u64vec4, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64A64_UINT; };
template<> struct _vk_format_for_type<glm::i8vec4, true> { static constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SNORM; };
template<> struct _vk_format_for_type<glm::i16vec4, true> { static constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SNORM; };
template<> struct _vk_format_for_type<glm::u8vec4, true> { static constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM; };
template<> struct _vk_format_for_type<glm::u16vec4, true> { static constexpr VkFormat format = VK_FORMAT_R16G16B16A16_UNORM; };
template<> struct _vk_format_for_type<glm::tvec4<half_float::half>, false> { static constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT; };
template<> struct _vk_format_for_type<glm::vec4, false> { static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT; };
template<> struct _vk_format_for_type<glm::f64vec4, false> { static constexpr VkFormat format = VK_FORMAT_R64G64B64A64_SFLOAT; };
}
template<typename T, bool normalized = false> struct vk_format_for_type {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto format = _detail::_vk_format_for_type<TypeT, normalized>::format;
};


// Type traits for image format (VkFormat)
template<VkFormat format>
struct vk_format_traits {};
template<> struct vk_format_traits<VK_FORMAT_R4G4_UNORM_PACK8> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_R4G4B4A4_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_B4G4R4A4_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_R5G6B5_UNORM_PACK16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_B5G6R5_UNORM_PACK16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_R5G5B5A1_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_B5G5R5A1_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A1R5G5B5_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_R8_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_SNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_USCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_SSCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8_SRGB> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_UNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_SNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_USCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_SSCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8_SRGB> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8_SRGB> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8_SRGB> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGR8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_R8G8B8A8_SRGB> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_UNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SNORM_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_USCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SSCALED_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_UINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SINT_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_B8G8R8A8_SRGB> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_BGRA8_SRGB_PACK8;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A8B8G8R8_SRGB_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool depth = false;
	using element_type = void;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A2R10G10B10_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_A2B10G10R10_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_format_traits<VK_FORMAT_R16_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_UNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_SNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_USCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_USCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_SSCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SSCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_UINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_R16_SFLOAT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_UNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_UNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_SNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_USCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_USCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_SSCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SSCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_UINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG16_SFLOAT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_UNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_USCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SSCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_UINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB16_SFLOAT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_UNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_USCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SSCALED_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_UINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SINT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R16G16B16A16_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = half_float::half;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA16_SFLOAT_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_R32_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_UINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_SINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_R32_SFLOAT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_UINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_SINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG32_SFLOAT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_UINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_SINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB32_SFLOAT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32A32_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_UINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32A32_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_SINT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R32G32B32A32_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA32_SFLOAT_PACK32;
};
template<> struct vk_format_traits<VK_FORMAT_R64_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_UINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_SINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_R64_SFLOAT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_UINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_SINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RG64_SFLOAT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_UINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_SINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGB64_SFLOAT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64A64_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_UINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64A64_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_SINT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_R64G64B64A64_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
	static constexpr gli::format gli_format = gli::format::FORMAT_RGBA64_SFLOAT_PACK64;
};
template<> struct vk_format_traits<VK_FORMAT_B10G11R11_UFLOAT_PACK32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_E5B9G9R9_UFLOAT_PACK32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_D16_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
	static constexpr gli::format gli_format = gli::format::FORMAT_D16_UNORM_PACK16;
};
template<> struct vk_format_traits<VK_FORMAT_X8_D24_UNORM_PACK32> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false;
};
template<> struct vk_format_traits<VK_FORMAT_D32_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = true,
		is_signed = true;
	using element_type = float;
	static constexpr gli::format gli_format = gli::format::FORMAT_D32_SFLOAT_PACK32;
};

}
}
