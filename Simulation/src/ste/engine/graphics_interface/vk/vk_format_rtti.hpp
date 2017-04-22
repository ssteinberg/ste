//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_format_type_traits.hpp>

#include <unordered_map>
#include <optional.hpp>

#include <stdexcept>

namespace StE {
namespace GL {

struct vk_format_rtti {
	int elements;
	int texel_bytes;
	bool is_depth;
	bool is_float;
	bool is_signed;
};

class vk_format_rtti_database {
private:
	static std::unordered_map<VkFormat, vk_format_rtti> database;

public:
	static optional<vk_format_rtti> get(const VkFormat &format) {
		auto it = database.find(format);
		if (it == database.end())
			return none;

		return it->second;
	}
};

/**
 *	@brief	Returns a vk_format_rtti structure (if available) describing the Vulkan image format
 *	
 *	@throws	std::runtime_error	If format not found
 */
vk_format_rtti inline vk_format_id(const VkFormat &format) {
	auto ret = vk_format_rtti_database::get(format);
	if (ret)
		return ret.get();

	throw std::runtime_error("format not found");
}

/**
*	@brief	Helper method for Vulkan image format's elements count query
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_elements(const VkFormat &format) {
	return vk_format_id(format).elements;
}

/**
*	@brief	Helper method for Vulkan image format's texel size (in bytes) query
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_texel_size(const VkFormat &format) {
	return vk_format_id(format).texel_bytes;
}

/**
*	@brief	Helper method, checks if a Vulkan image format has a depth aspect
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_is_depth(const VkFormat &format) {
	return vk_format_id(format).is_depth;
}

/**
*	@brief	Helper method, classifies the Vulkan image format's main aspect.
*	
*	@return	VK_IMAGE_ASPECT_DEPTH_BIT for depth formats, VK_IMAGE_ASPECT_COLOR_BIT for color formats.
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_aspect(const VkFormat &format) {
	return vk_format_is_depth(format) ? 
		VK_IMAGE_ASPECT_DEPTH_BIT : 
		VK_IMAGE_ASPECT_COLOR_BIT;
}

/**
*	@brief	Helper method, checks if a Vulkan image format is a floating-point format
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_is_float(const VkFormat &format) {
	return vk_format_id(format).is_float;
}

/**
*	@brief	Helper method, checks if a Vulkan image format is signed or unsigned
*
*	@throws	std::runtime_error	If format not found
*/
auto inline vk_format_is_signed(const VkFormat &format) {
	return vk_format_id(format).is_signed;
}

}
}
