//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>
#include <image_aspect.hpp>

#include <unordered_map>
#include <optional.hpp>

#include <stdexcept>

namespace ste {
namespace gl {

struct format_rtti {
	int elements;
	int texel_bytes;
	bool is_depth;
	bool is_float;
	bool is_signed;
	bool is_srgb;
	bool is_normalized_integer;
	bool is_scaled_integer;
	gli::format gli_format;
};

namespace _internal {

class format_rtti_database {
private:
	static std::unordered_map<format, format_rtti> database;

public:
	static optional<format_rtti> get(const format &format) {
		auto it = database.find(format);
		if (it == database.end())
			return none;

		return it->second;
	}
};

}

/**
 *	@brief	Returns a format_rtti structure (if available) describing the Vulkan image format
 *	
 *	@throws	std::runtime_error	If format not found
 */
format_rtti inline format_id(const format &format) {
	auto ret = _internal::format_rtti_database::get(format);
	if (ret)
		return ret.get();

	throw std::runtime_error("format not found");
}

/**
*	@brief	Helper method for Vulkan image format's elements count query
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_elements(const format &format) {
	return format_id(format).elements;
}

/**
*	@brief	Helper method for Vulkan image format's texel size (in bytes) query
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_texel_size(const format &format) {
	return format_id(format).texel_bytes;
}

/**
*	@brief	Helper method, checks if a Vulkan image format has a depth aspect
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_is_depth(const format &format) {
	return format_id(format).is_depth;
}

/**
*	@brief	Helper method, classifies the Vulkan image format's main aspect.
*	
*	@return	image_aspect::depth for depth formats, image_aspect::color for color formats.
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_aspect(const format &format) {
	return format_is_depth(format) ? 
		image_aspect::depth :
		image_aspect::color;
}

/**
*	@brief	Helper method, checks if a Vulkan image format is a floating-point format
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_is_float(const format &format) {
	return format_id(format).is_float;
}

/**
*	@brief	Helper method, checks if a Vulkan image format is signed or unsigned
*
*	@throws	std::runtime_error	If format not found
*/
auto inline format_is_signed(const format &format) {
	return format_id(format).is_signed;
}

}
}
