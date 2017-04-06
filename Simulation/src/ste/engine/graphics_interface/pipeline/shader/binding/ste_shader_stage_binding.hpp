// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding_variable.hpp>
#include <ste_shader_variable_layout_verification_exceptions.hpp>
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
	template <int a, typename... Ts>
	void validate_layout_impl(const block_layout<a, Ts...> &block) const {
		// Array blocks are always encapsulated in a single member struct
		auto struct_ptr = dynamic_cast<const ste_shader_stage_binding_variable_struct*>(variable.get());
		if (struct_ptr &&
			struct_ptr->count() == 1) {
			const ste_shader_stage_binding_variable *var = (*struct_ptr)[0].get();

			// If var is an array, we are only interested in the underlying type
			auto array_ptr = dynamic_cast<const ste_shader_stage_binding_variable_array*>(var);
			if (array_ptr) {
				array_ptr->underlying_variable()->validate<>(block);
				return;
			}
		}

		// Non-array block
		variable->validate<>(block);
	}

public:
	std::uint32_t set_idx;
	std::uint32_t bind_idx;

	ste_shader_stage_binding_type binding_type{ ste_shader_stage_binding_type::unknown };
	ste_shader_stage_block_layout block_layout;

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
	template <typename... Ts>
	void validate_layout(const std140_layout<Ts...> &block) const {
		if (block_layout != ste_shader_stage_block_layout::std140)
			throw ste_shader_variable_layout_verification_block_layout_mismatch("Block layout mismatch");
		validate_layout_impl(block);
	}
	/**
	*	@brief	Validate a block layout.
	*	
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename... Ts>
	void validate_layout(const std430_layout<Ts...> &block) const {
		if (block_layout != ste_shader_stage_block_layout::std430)
			throw ste_shader_variable_layout_verification_block_layout_mismatch("Block layout mismatch");
		validate_layout_impl(block);
	}
};


}
}
