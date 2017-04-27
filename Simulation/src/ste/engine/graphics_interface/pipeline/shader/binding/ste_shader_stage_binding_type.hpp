// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable_type.hpp>
#include <ste_shader_exceptions.hpp>

namespace ste {
namespace gl {

enum class ste_shader_stage_binding_type : std::uint16_t {
	unknown,
	uniform,
	storage,
	push_constant,
	spec_constant,
};

/**
*	@brief	Returns the appropriate VkDescriptorType type.
*
*	@throws	ste_shader_binding_incompatible_type	If variable type isn't a block or image/texture/sampler
*	@throws	ste_shader_binding_specialization_or_push_constant_exception	If variable isn't binding_type isn't storage or uniform
*/
VkDescriptorType inline vk_descriptor_for_binding(const ste_shader_stage_binding_type &binding_type,
												  const ste_shader_stage_variable_type &variable_type) {
	if (binding_type == ste_shader_stage_binding_type::uniform) {
		switch (variable_type) {
		case ste_shader_stage_variable_type::image_t:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case ste_shader_stage_variable_type::storage_image_t:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case ste_shader_stage_variable_type::texture_t:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case ste_shader_stage_variable_type::sampler_t:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case ste_shader_stage_variable_type::int_t:
		case ste_shader_stage_variable_type::uint_t:
		case ste_shader_stage_variable_type::float_t:
		case ste_shader_stage_variable_type::bool_t:
		case ste_shader_stage_variable_type::struct_t:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		default:
			throw ste_shader_binding_incompatible_type("Unexpected type");
		}
	}

	if (binding_type == ste_shader_stage_binding_type::storage &&
		variable_type == ste_shader_stage_variable_type::struct_t) {
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}

	assert(binding_type != ste_shader_stage_binding_type::storage && "Storage but not struct?!");
	throw ste_shader_binding_specialization_or_push_constant_exception("Variable is a specialization constant or push constant");
}

}
}
