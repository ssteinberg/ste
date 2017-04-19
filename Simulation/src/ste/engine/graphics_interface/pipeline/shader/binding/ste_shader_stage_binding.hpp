// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding_variable.hpp>
#include <ste_shader_stage_binding_type.hpp>
#include <ste_shader_variable_layout_verification_exceptions.hpp>
#include <vk_descriptor_set_layout_binding.hpp>
#include <pipeline_layout_set_index.hpp>

#include <memory>

#include <std140_layout.hpp>
#include <std430_layout.hpp>

namespace StE {
namespace GL {

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
			auto struct_var = (*struct_ptr)[0].get();
			auto array_ptr = dynamic_cast<const ste_shader_stage_binding_variable_array*>(struct_var);
			if (array_ptr)
				var = array_ptr;
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
		return vk_descriptor_for_binding(binding_type,
										 variable->type());
	}
};

}
}
