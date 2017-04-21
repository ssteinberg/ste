//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_layout_interface.hpp>
#include <pipeline_layout_set_index.hpp>

#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage_binding_type.hpp>
#include <ste_shader_stage_variable_type.hpp>
#include <pipeline_binding_stages_collection.hpp>

#include <ste_shader_stage_variable_from_type.hpp>
#include <std430_layout.hpp>

#include <functional>
#include <type_traits>
#include <string>
#include <memory>

namespace StE {
namespace GL {

/**
*	@brief	External binding descriptor
*	
*	@param	T		Binding type
*	@param	set		Set index
*	@param	bind		Binding index
*	@param	storage_image	For images only, true for storage image, false for sampled image
*	@param	push_constant	Declares a push constant
*/
template <
	class T,
	pipeline_layout_set_index set, 
	std::uint32_t bind,
	bool storage_image = false
>
class pipeline_external_binding_descriptor {
public:
	using type = T;
	static constexpr bool is_storage_image = storage_image;

	static constexpr auto set_idx = set;
	static constexpr auto bind_idx = bind;

	static constexpr ste_shader_stage_variable_type var_type = ste_shader_stage_variable_type_from_type_v<T, storage_image>;

	static_assert(var_type != ste_shader_stage_variable_type::unknown, "Type can not be unknown");

	static ste_shader_stage_variable_type variable_type() {
		return var_type;
	}
	static ste_shader_stage_binding_type binding_type() {
		return is_std430_block_layout_v<T> ?
			ste_shader_stage_binding_type::storage :
			ste_shader_stage_binding_type::uniform;
	}
	static ste_shader_stage_block_layout block_layout() {
		if (is_std430_block_layout_v<T>)
			return ste_shader_stage_block_layout::std430;
		if (is_std140_block_layout_v<T>)
			return ste_shader_stage_block_layout::std140;
		return ste_shader_stage_block_layout::none;
	}
};

/**
*	@brief	Describes an external binding's layout
*/
class pipeline_external_binding_layout : public pipeline_binding_layout_interface {
private:
	using validator_t = std::function<void(const ste_shader_stage_binding*)>;

private:
	pipeline_binding_stages_collection binding_stages;

	ste_shader_stage_binding binding;
	validator_t validator;

private:
	// Type erases the validation
	template <typename T>
	static auto create_validator() {
		return validator_t([](const ste_shader_stage_binding *binding) {
			binding->validate_layout<T>();
		});
	}

	template <pipeline_layout_set_index s, std::uint32_t b, class T, bool si>
	static auto create_binding(const std::string &name,
							   const pipeline_external_binding_descriptor<T, s, b, si> &descriptor) {
		ste_shader_stage_binding binding;
		binding.bind_idx = descriptor.bind_idx;
		binding.set_idx = descriptor.set_idx;
		binding.binding_type = descriptor.binding_type();
		binding.block_layout = descriptor.block_layout();

		std::unique_ptr<ste_shader_stage_binding_variable> variable = 
			ste_shader_stage_variable_from_type<T, descriptor.is_storage_image>(name);
		binding.variable = std::move(variable);

		return binding;
	}

public:
	template <pipeline_layout_set_index s, std::uint32_t b, class T, bool si>
	pipeline_external_binding_layout(const std::string &name,
									 const pipeline_binding_stages_collection &stages,
									 const pipeline_external_binding_descriptor<T, s, b, si> &descriptor)
		: binding_stages(stages),
		binding(create_binding(name,
							   descriptor)),
		validator(create_validator<T>())
	{}
	~pipeline_external_binding_layout() noexcept {}

	pipeline_external_binding_layout(pipeline_external_binding_layout&&) = default;
	pipeline_external_binding_layout &operator=(pipeline_external_binding_layout&&) = default;

	/**
	*	@brief	Checks the compatibility of binding the external binding to a shader stage binding.
	*
	*	@param	binding		A shader stage binding that does the validation
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	void validate_layout(const ste_shader_stage_binding *binding) const {
		validator(binding);
	}

	/**
	*	@brief	Validates the layout of a type, checking its compatibility with the external binding.
	*
	*	@throws	ste_shader_variable_layout_verification_exception	On different validation failures
	*/
	template <typename T>
	void validate_layout() const {
		binding.validate_layout<T>();
	}

	const std::string &name() const override { return binding.variable->name(); }
	pipeline_layout_set_index set_idx() const override { return binding.set_idx; }
	std::uint32_t bind_idx() const override { return binding.bind_idx; }
	ste_shader_stage_binding_type binding_type() const override { return binding.binding_type; }
	std::uint32_t count() const override { return binding.variable->size(); }
	const pipeline_binding_stages_collection& stage_collection() const override { return binding_stages; }

	/**
	*	@brief	Returns the appropriate VkDescriptorType type.
	*
	*	@throws	ste_shader_binding_incompatible_type	If variable type isn't a block or image/texture/sampler
	*	@throws	ste_shader_binding_specialization_or_push_constant_exception	If variable isn't binding_type isn't storage or uniform
	*/
	VkDescriptorType vk_descriptor_type() const override {
		return vk_descriptor_for_binding(binding_type(),
										 binding.variable->type());
	}
};

}
}
