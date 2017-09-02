//	StE
// © Shlomi Steinberg 2015-2017

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

// Run-time image dimensions type trait
inline std::uint32_t image_dimensions_for_type(image_type type) {
	switch (type) {
	default:
		assert(false);
	case image_type::image_1d:
			return 1;
	case image_type::image_2d:
			return 2;
	case image_type::image_3d:
			return 3;
	case image_type::image_1d_array:
			return 1;
	case image_type::image_2d_array:
			return 2;
	case image_type::image_cubemap:
			return 2;
	case image_type::image_cubemap_array:
			return 2;
	}
}

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

// Run-time image Vulkan type (VkImageViewType) type trait
inline VkImageViewType image_vk_type_for_type(image_type type) {
	switch (type) {
	default:
		assert(false);
	case image_type::image_1d:
			return VK_IMAGE_VIEW_TYPE_1D;
	case image_type::image_2d:
			return VK_IMAGE_VIEW_TYPE_2D;
	case image_type::image_3d:
			return VK_IMAGE_VIEW_TYPE_3D;
	case image_type::image_1d_array:
			return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case image_type::image_2d_array:
			return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case image_type::image_cubemap:
			return VK_IMAGE_VIEW_TYPE_CUBE;
	case image_type::image_cubemap_array:
			return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	}
}

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

// Run-time image type trait - image arrays
inline bool image_has_arrays_for_type(image_type type) {
	switch (type) {
	default:
		assert(false);
	case image_type::image_1d:
			return false;
	case image_type::image_2d:
			return false;
	case image_type::image_3d:
			return false;
	case image_type::image_1d_array:
			return true;
	case image_type::image_2d_array:
			return true;
	case image_type::image_cubemap:
			return false;
	case image_type::image_cubemap_array:
			return true;
	}
}

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

// Run-time image type trait - cubemap images
inline bool image_is_cubemap_for_type(image_type type) {
	switch (type) {
	default:
		assert(false);
	case image_type::image_1d:
			return false;
	case image_type::image_2d:
			return false;
	case image_type::image_3d:
			return false;
	case image_type::image_1d_array:
			return false;
	case image_type::image_2d_array:
			return false;
	case image_type::image_cubemap:
			return true;
	case image_type::image_cubemap_array:
			return true;
	}
}

// Image's dimension type trait - Extent type
template<std::uint32_t dimensions>
struct image_extent_type {};
template <>
struct image_extent_type<1> {
	using type = glm::u32vec1;
};
template <>
struct image_extent_type<2> {
	using type = glm::u32vec2;
};
template <>
struct image_extent_type<3> {
	using type = glm::u32vec3;
};
template <std::uint32_t dimensions>
using image_extent_type_t = typename image_extent_type<dimensions>::type;

// Image type trait - layer image type
template<image_type type>
struct image_layer_type {};
template <>
struct image_layer_type<image_type::image_1d> {
	static constexpr auto value = image_type::image_1d;
};
template <>
struct image_layer_type<image_type::image_1d_array> {
	static constexpr auto value = image_type::image_1d;
};
template <>
struct image_layer_type<image_type::image_2d> {
	static constexpr auto value = image_type::image_2d;
};
template <>
struct image_layer_type<image_type::image_2d_array> {
	static constexpr auto value = image_type::image_2d;
};
template <>
struct image_layer_type<image_type::image_3d> {
	static constexpr auto value = image_type::image_3d;
};
template <>
struct image_layer_type<image_type::image_cubemap> {
	static constexpr auto value = image_type::image_cubemap;
};
template <>
struct image_layer_type<image_type::image_cubemap_array> {
	static constexpr auto value = image_type::image_cubemap;
};
template <image_type type>
static constexpr auto image_layer_type_v = image_layer_type<type>::value;

// Run-time image type trait - layer image type
inline image_type image_layer_type_for_type(image_type type) {
	switch (type) {
	default:
		assert(false);
	case image_type::image_1d:
			return image_type::image_1d;
	case image_type::image_1d_array:
			return image_type::image_1d;
	case image_type::image_2d:
			return image_type::image_2d;
	case image_type::image_2d_array:
			return image_type::image_2d;
	case image_type::image_3d:
			return image_type::image_3d;
	case image_type::image_cubemap:
			return image_type::image_cubemap;
	case image_type::image_cubemap_array:
			return image_type::image_cubemap;
	}
}

}
}
