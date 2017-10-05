//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>
#include <image_aspect.hpp>

#include <surface_block_load.hpp>

#include <unordered_map>
#include <optional.hpp>

#include <stdexcept>

namespace ste {
namespace gl {

struct format_rtti {
	std::uint8_t elements;
	std::uint8_t block_bytes;
	glm::u8vec2 block_extent;

	unsigned is_depth : 1;
	unsigned is_float : 1;
	unsigned is_signed : 1;
	unsigned is_srgb : 1;
	unsigned is_normalized_integer : 1;
	unsigned is_scaled_integer : 1;
	unsigned is_compressed : 1;

	// Name of block common type
	resource::block_common_type block_common_type_name;

	using _block_loader_func_ptr = std::size_t(*)(const std::uint8_t *input, void *output);
	using _block_loader_8component_func_ptr = void(*)(const std::uint8_t *input, unsigned count);

	// Loader function pointers. Before use, must be cast to correct type based on common_type.
	_block_loader_func_ptr block_loader{ nullptr };
	_block_loader_8component_func_ptr block_loader_8component_r{ nullptr };
	_block_loader_8component_func_ptr block_loader_8component_g{ nullptr };
	_block_loader_8component_func_ptr block_loader_8component_b{ nullptr };
	_block_loader_8component_func_ptr block_loader_8component_a{ nullptr };
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
	return byte_t(format_id(format).block_bytes);
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

/**
*	@brief	Helper method, returns the block loader function pointer for type fp32
*/
auto inline format_block_loader_fp32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::fp32);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, float *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block loader function pointer for type fp64
*/
auto inline format_block_loader_fp64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::fp64);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, double *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block loader function pointer for type i32
*/
auto inline format_block_loader_i32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::int32);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, std::int32_t *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block loader function pointer for type u32
*/
auto inline format_block_loader_u32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::uint32);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, std::uint32_t *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block loader function pointer for type i64
*/
auto inline format_block_loader_i64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::int64);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, std::int64_t *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block loader function pointer for type u64
*/
auto inline format_block_loader_u64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::uint64);
	return reinterpret_cast<std::size_t(*)(const std::uint8_t *, std::uint64_t *)>(fr.block_loader);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type fp32
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_fp32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::fp32);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<float>(*)(const std::uint8_t *, unsigned)>(ptr);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type fp64
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_fp64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::fp64);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<double>(*)(const std::uint8_t *, unsigned)>(ptr);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type i32
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_i32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::int32);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<std::int32_t>(*)(const std::uint8_t *, unsigned)>(ptr);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type u32
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_u32(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::uint32);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<std::uint32_t>(*)(const std::uint8_t *, unsigned)>(ptr);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type i64
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_i64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::int64);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<std::int64_t>(*)(const std::uint8_t *, unsigned)>(ptr);
}

/**
*	@brief	Helper method, returns the block 8-component loader function pointer for type u64
*/
template <gl::component_swizzle comp>
auto format_block_loader_8component_u64(const format_rtti &fr) {
	assert(fr.block_common_type_name == resource::block_common_type::uint64);

	const auto ptr =
		comp == component_swizzle::r ? fr.block_loader_8component_r :
		(comp == component_swizzle::g ? fr.block_loader_8component_g :
		(comp == component_swizzle::b ? fr.block_loader_8component_b : fr.block_loader_8component_a));
	return reinterpret_cast<resource::block_load_8component_result_t<std::uint64_t>(*)(const std::uint8_t *, unsigned)>(ptr);
}

}
}
