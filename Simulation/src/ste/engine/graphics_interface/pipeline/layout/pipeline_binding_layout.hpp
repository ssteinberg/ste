//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_layout_interface.hpp>
#include <ste_shader_stage_binding.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Fully describes a single resource binding end-point for multiple stages in a pipeline
 */
struct pipeline_binding_layout : pipeline_binding_layout_interface {
	const ste_shader_stage_binding* binding;
	pipeline_binding_stages_collection stages;

	/**
	*	@brief	Helper method to get underlying variable name
	*/
	const lib::string& name() const override { return binding->variable->name(); }

	/**
	*	@brief	Helper method to get the binding's binding index
	*/
	std::uint32_t bind_idx() const override { return binding->bind_idx; }

	/**
	*	@brief	Helper method to get the binding's set index
	*/
	pipeline_layout_set_index set_idx() const override { return binding->set_idx; }

	/**
	*	@brief	Helper method to get the binding's Vulkan descriptor type
	*/
	VkDescriptorType vk_descriptor_type() const override { return *binding; }

	/**
	*	@brief	Helper method to get the binding's Vulkan count, i.e. underlying variable's array element count
	*/
	std::uint32_t count() const override { return binding->variable->size(); }

	/**
	*	@brief	Helper method to get the binding's type
	*/
	ste_shader_stage_binding_type binding_type() const override { return binding->binding_type; }

	/**
	*	@brief	Helper method to get the binding's stages collection
	*/
	const pipeline_binding_stages_collection& stage_collection() const override {
		return stages;
	}
};

}
}
