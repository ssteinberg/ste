//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <image_type.hpp>

namespace ste {
namespace gl {

// Image dimensions type trait
template<image_type type>
struct image_dimensions {};
template <>
struct image_dimensions<image_type::image_1d> {
	static constexpr std::uint32_t value = 1;
};
template <>
struct image_dimensions<image_type::image_2d> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct image_dimensions<image_type::image_3d> {
	static constexpr std::uint32_t value = 3;
};
template <>
struct image_dimensions<image_type::image_1d_array> {
	static constexpr std::uint32_t value = 1;
};
template <>
struct image_dimensions<image_type::image_2d_array> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct image_dimensions<image_type::image_cubemap> {
	static constexpr std::uint32_t value = 2;
};
template <>
struct image_dimensions<image_type::image_cubemap_array> {
	static constexpr std::uint32_t value = 2;
};
template <image_type type>
static constexpr auto image_dimensions_v = image_dimensions<type>::value;

// Image Vulkan type (VkImageViewType) type trait
template<image_type type>
struct image_vk_type {};
template <>
struct image_vk_type<image_type::image_1d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_1D;
};
template <>
struct image_vk_type<image_type::image_2d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_2D;
};
template <>
struct image_vk_type<image_type::image_3d> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_3D;
};
template <>
struct image_vk_type<image_type::image_1d_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
};
template <>
struct image_vk_type<image_type::image_2d_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
};
template <>
struct image_vk_type<image_type::image_cubemap> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_CUBE;
};
template <>
struct image_vk_type<image_type::image_cubemap_array> {
	static constexpr VkImageViewType value = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
};
template <image_type type>
static constexpr auto image_vk_type_v = image_vk_type<type>::value;

// Image type trait - image arrays
template<image_type type>
struct image_has_arrays {};
template <>
struct image_has_arrays<image_type::image_1d> {
	static constexpr bool value = false;
};
template <>
struct image_has_arrays<image_type::image_2d> {
	static constexpr bool value = false;
};
template <>
struct image_has_arrays<image_type::image_3d> {
	static constexpr bool value = false;
};
template <>
struct image_has_arrays<image_type::image_1d_array> {
	static constexpr bool value = true;
};
template <>
struct image_has_arrays<image_type::image_2d_array> {
	static constexpr bool value = true;
};
template <>
struct image_has_arrays<image_type::image_cubemap> {
	static constexpr bool value = false;
};
template <>
struct image_has_arrays<image_type::image_cubemap_array> {
	static constexpr bool value = true;
};
template <image_type type>
static constexpr auto image_has_arrays_v = image_has_arrays<type>::value;

// Image type trait - cubemap images
template<image_type type>
struct image_is_cubemap {};
template <>
struct image_is_cubemap<image_type::image_1d> {
	static constexpr bool value = false;
};
template <>
struct image_is_cubemap<image_type::image_2d> {
	static constexpr bool value = false;
};
template <>
struct image_is_cubemap<image_type::image_3d> {
	static constexpr bool value = false;
};
template <>
struct image_is_cubemap<image_type::image_1d_array> {
	static constexpr bool value = false;
};
template <>
struct image_is_cubemap<image_type::image_2d_array> {
	static constexpr bool value = false;
};
template <>
struct image_is_cubemap<image_type::image_cubemap> {
	static constexpr bool value = true;
};
template <>
struct image_is_cubemap<image_type::image_cubemap_array> {
	static constexpr bool value = true;
};
template <image_type type>
static constexpr auto image_is_cubemap_v = image_is_cubemap<type>::value;

// Image's dimension type trait - Extent type
template<int dimensions>
struct image_extent_type {};
template <>
struct image_extent_type<1> {
	using type = glm::u32;
};
template <>
struct image_extent_type<2> {
	using type = glm::u32vec2;
};
template <>
struct image_extent_type<3> {
	using type = glm::u32vec3;
};
template <int dimensions>
using image_extent_type_t = typename image_extent_type<dimensions>::type;

}
}
