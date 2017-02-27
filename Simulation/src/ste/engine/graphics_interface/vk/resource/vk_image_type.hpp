//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

enum class vk_image_type {
	image_1d,
	image_1d_array,
	image_2d,
	image_2d_array,
	image_3d,
	image_cubemap,
	image_cubemap_array,
};

template<vk_image_type type>
struct vk_image_dimensions {};
template <>
struct vk_image_dimensions<vk_image_type::image_1d> {
	static constexpr std::uint32_t value = 1;
};
template <>
struct vk_image_dimensions<vk_image_type::image_2d> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct vk_image_dimensions<vk_image_type::image_3d> {
	static constexpr std::uint32_t value = 3;
};
template <>
struct vk_image_dimensions<vk_image_type::image_1d_array> {
	static constexpr std::uint32_t value = 1;
};
template <>
struct vk_image_dimensions<vk_image_type::image_2d_array> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct vk_image_dimensions<vk_image_type::image_cubemap> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct vk_image_dimensions<vk_image_type::image_cubemap_array> {
	static constexpr std::uint32_t value = 2;
};

template<vk_image_type type>
struct vk_image_vk_type {};
template <>
struct vk_image_vk_type<vk_image_type::image_1d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_1D;
};
template <>
struct vk_image_vk_type<vk_image_type::image_2d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_2D;
};
template <>
struct vk_image_vk_type<vk_image_type::image_3d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_3D;
};
template <>
struct vk_image_vk_type<vk_image_type::image_1d_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
};
template <>
struct vk_image_vk_type<vk_image_type::image_2d_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
};
template <>
struct vk_image_vk_type<vk_image_type::image_cubemap> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_CUBE;
};
template <>
struct vk_image_vk_type<vk_image_type::image_cubemap_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
};

template<int dimensions>
struct vk_image_extent_type {};
template <>
struct vk_image_extent_type<1> {
	using type = glm::u32;
};
template <>
struct vk_image_extent_type<2> {
	using type = glm::u32vec2;
};
template <>
struct vk_image_extent_type<3> {
	using type = glm::u32vec3;
};

template<VkImageType type>
struct vk_image_type_traits {};
template<> struct vk_image_type_traits<VK_FORMAT_R4G4_UNORM_PACK8> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_R4G4B4A4_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_B4G4R4A4_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_R5G6B5_UNORM_PACK16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_B5G6R5_UNORM_PACK16> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_R5G5B5A1_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_B5G5R5A1_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A1R5G5B5_UNORM_PACK16> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_SNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_USCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_SSCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8_SRGB> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 1;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_UNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_SNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_USCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_SSCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8_SRGB> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8_SRGB> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8_SRGB> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 3;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R8G8B8A8_SRGB> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_B8G8R8A8_SRGB> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A8B8G8R8_SRGB_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint8_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool depth = false;
	using element_type = void;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2R10G10B10_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_UNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_SNORM_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_USCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_SSCALED_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_UINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_A2B10G10R10_SINT_PACK32> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_SNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_USCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_SSCALED> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_UNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_SNORM> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_USCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_SSCALED> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_UNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_SNORM> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_USCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_SSCALED> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 6;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_UNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_SNORM> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_USCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_SSCALED> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R16G16B16A16_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 12;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32A32_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32A32_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int32_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R32G32B32A32_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = float;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64_UINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64_SINT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 8;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64_UINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64_SINT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64_SFLOAT> {
	static constexpr int elements = 2;
	static constexpr int texel_bytes = 16;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64_UINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64_SINT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64_SFLOAT> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 24;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64A64_UINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = false;
	using element_type = std::uint64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64A64_SINT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = false,
		is_signed = true;
	using element_type = std::int64_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_R64G64B64A64_SFLOAT> {
	static constexpr int elements = 4;
	static constexpr int texel_bytes = 32;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = true;
	using element_type = double;
};
template<> struct vk_image_type_traits<VK_FORMAT_B10G11R11_UFLOAT_PACK32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_E5B9G9R9_UFLOAT_PACK32> {
	static constexpr int elements = 3;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = false,
		is_float = true,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_D16_UNORM> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 2;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false;
	using element_type = std::uint16_t;
};
template<> struct vk_image_type_traits<VK_FORMAT_X8_D24_UNORM_PACK32> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = false,
		is_signed = false;
};
template<> struct vk_image_type_traits<VK_FORMAT_D32_SFLOAT> {
	static constexpr int elements = 1;
	static constexpr int texel_bytes = 4;
	static constexpr bool is_depth = true,
		is_float = true,
		is_signed = true;
	using element_type = float;
};

}
}
