//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding.hpp>
#include <pipeline_binding_stages_collection.hpp>

#include <vk_descriptor_set_layout_binding.hpp>

#include <hash_combine.hpp>

namespace StE {
namespace GL {

/**
 *	@brief	Fully describes a single resource binding end-point for multiple stages in a pipeline
 */
struct pipeline_binding_set_layout_binding {
	const ste_shader_stage_binding* binding;
	pipeline_binding_stages_collection stages;

	/**
	*	@brief	Helper method to get underlying variable name
	*/
	auto& name() const { return binding->variable->name(); }

	/**
	*	@brief	Helper method to get the binding's binding index
	*/
	auto& bind_idx() const { return binding->bind_idx; }

	/**
	*	@brief	Helper method to get the binding's set index
	*/
	auto& set_idx() const { return binding->set_idx; }

	/**
	*	@brief	Helper method to get the binding's Vulkan descriptor type
	*/
	VkDescriptorType vk_descriptor_type() const { return *binding; }

	/**
	*	@brief	Helper method to get the binding's Vulkan count, i.e. underlying variable's array element count
	*/
	auto count() const { return binding->variable->size(); }

	/**
	 *	@brief	Creates the Vulkan binding descriptor
	 */
	operator vk_descriptor_set_layout_binding() const {
		return vk_descriptor_set_layout_binding(*binding,
												stages,
												binding->bind_idx,
												count());
	}
};

}
}
