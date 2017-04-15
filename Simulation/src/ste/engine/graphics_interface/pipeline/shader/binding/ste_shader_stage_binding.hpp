// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding_variable.hpp>
#include <ste_shader_variable_layout_verification_exceptions.hpp>
#include <vk_descriptor_set_layout_binding.hpp>
#include <ste_shader_exceptions.hpp>
#include <pipeline_layout_set_index.hpp>

#include <memory>

#include <std140_layout.hpp>
#include <std430_layout.hpp>

namespace StE {
namespace GL {

enum class ste_shader_stage_binding_type : std::uint16_t {
	unknown,
	uniform,
	storage,
	push_constant,
	spec_constant,
};

enum class ste_shader_stage_block_layout : std::uint16_t {
	none,
	std140,
	std430,
};

struct ste_shader_stage_binding {
private:
	template <typename T>
	static void validate_layout_impl(const ste_shader_stage_binding_variable *var) {
		// If var is an array, we are only interested in the underlying type
		auto array_ptr = dynamic_cast<const ste_shader_stage_binding_variable_array*>(var);
		if (array_ptr) {
			array_ptr->underlying_variable()->validate<T>();
		}
		else {
			// Non-array variable
			var->validate<T>();
		}
	}

	template <typename BlockType>
	void validate_layout_block() const {
		static_assert(is_block_layout_v<BlockType>, "Expected a block layout");

		const ste_shader_stage_binding_variable *var = variable.get();

		// Array blocks usually are encapsulated in a single member struct
		auto struct_ptr = dynamic_cast<const ste_shader_stage_binding_variable_struct*>(var);
		if (struct_ptr &&
			struct_ptr->count() == 1) {
			var = (*struct_ptr)[0].get();
		}

		validate_layout_impl<BlockType>(var);
	}

public:
	pipeline_layout_set_index set_idx;
	std::uint32_t bind_idx;

	ste_shader_stage_binding_type binding_type{ ste_shader_stage_binding_type::unknown };
	ste_shader_stage_block_layout block_layout{ ste_shader_stage_block_layout::none };

	std::unique_ptr<ste_shader_stage_binding_variable> variable;

	/**
	*	@brief	Returns true if the binding references am array and the array length is a specializeable constant.
	*/
	bool is_array_length_specializeable() const {
		auto array_ptr = dynamic_cast<const ste_shader_stage_binding_variable_array*>(variable.get());
		return array_ptr &&
			array_ptr->length_spec_constant();
	}

	/**
	*	@brief	Validate a block layout.
	*	
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate_layout(std::enable_if_t<is_std140_block_layout_v<T>>* = nullptr) const {
		if (block_layout != ste_shader_stage_block_layout::std140)
			throw ste_shader_variable_layout_verification_block_layout_mismatch("Block layout mismatch");
		validate_layout_block<T>();
	}
	/**
	*	@brief	Validate a block layout.
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate_layout(std::enable_if_t<is_std430_block_layout_v<T>>* = nullptr) const {
		if (block_layout != ste_shader_stage_block_layout::std430)
			throw ste_shader_variable_layout_verification_block_layout_mismatch("Block layout mismatch");
		validate_layout_block<T>();
	}
	/**
	*	@brief	Validate a layout.
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate_layout(std::enable_if_t<!is_block_layout_v<T>>* = nullptr) const {
		if (block_layout != ste_shader_stage_block_layout::none)
			throw ste_shader_variable_layout_verification_block_layout_mismatch("Expected a block layout");
		validate_layout_impl<T>(variable.get());
	}

	/**
	*	@brief	Returns the appropriate VkDescriptorType type.
	*
	*	@throws	ste_shader_binding_incompatible_type	If variable type isn't a block or image/texture/sampler
	*	@throws	ste_shader_binding_specialization_or_push_constant_exception	If variable isn't binding_type isn't storage or uniform
	*/
	operator VkDescriptorType() const {
		if (binding_type == ste_shader_stage_binding_type::uniform) {
			switch (variable->type()) {
			case ste_shader_stage_variable_type::struct_t:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case ste_shader_stage_variable_type::image_t:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case ste_shader_stage_variable_type::storage_image_t:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case ste_shader_stage_variable_type::texture_t:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case ste_shader_stage_variable_type::sampler_t:
				return VK_DESCRIPTOR_TYPE_SAMPLER;
			default:
				throw ste_shader_binding_incompatible_type("Expected an opaque type");
			}
		}
		
		if (binding_type == ste_shader_stage_binding_type::storage &&
			variable->type() == ste_shader_stage_variable_type::struct_t) {
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}

		assert(binding_type != ste_shader_stage_binding_type::storage && "Storage but not struct?!");
		throw ste_shader_binding_specialization_or_push_constant_exception("Variable is a specialization constant or push constant");
	}
};


}
}
