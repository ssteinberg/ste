//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_image_type.hpp>

namespace StE {
namespace GL {

// Image dimensions type trait
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

// Image Vulkan type (VkImageViewType) type trait
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

// Image type trait - image arrays
template<vk_image_type type>
struct vk_image_has_arrays {};
template <>
struct vk_image_has_arrays<vk_image_type::image_1d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_2d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_3d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_1d_array> {
	static constexpr bool value = true;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_2d_array> {
	static constexpr bool value = true;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_cubemap> {
	static constexpr bool value = false;
};
template <>
struct vk_image_has_arrays<vk_image_type::image_cubemap_array> {
	static constexpr bool value = true;
};

// Image type trait - cubemap images
template<vk_image_type type>
struct vk_image_is_cubemap {};
template <>
struct vk_image_is_cubemap<vk_image_type::image_1d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_2d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_3d> {
	static constexpr bool value = false;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_1d_array> {
	static constexpr bool value = false;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_2d_array> {
	static constexpr bool value = false;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_cubemap> {
	static constexpr bool value = true;
};
template <>
struct vk_image_is_cubemap<vk_image_type::image_cubemap_array> {
	static constexpr bool value = true;
};

// Image's dimension type trait - Extent type
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

}
}
