//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_stages_collection.hpp>

#include <pipeline_layout_set_index.hpp>
#include <vk_descriptor_set_layout_binding.hpp>

#include <string>

namespace ste {
namespace gl {

struct pipeline_binding_layout_interface {
	virtual ~pipeline_binding_layout_interface() noexcept {}

	/**
	*	@brief	Helper method to get underlying variable name
	*/
	virtual const std::string& name() const = 0;

	/**
	*	@brief	Helper method to get the binding's binding index
	*/
	virtual std::uint32_t bind_idx() const = 0;

	/**
	*	@brief	Helper method to get the binding's set index
	*/
	virtual pipeline_layout_set_index set_idx() const = 0;

	/**
	*	@brief	Helper method to get the binding's Vulkan descriptor type
	*/
	virtual VkDescriptorType vk_descriptor_type() const = 0;

	/**
	*	@brief	Helper method to get the binding's Vulkan count, i.e. underlying variable's array element count
	*/
	virtual std::uint32_t count() const = 0;

	/**
	*	@brief	Helper method to get the binding's type
	*/
	virtual ste_shader_stage_binding_type binding_type() const = 0;

	/**
	*	@brief	Helper method to get the binding's stages collection
	*/
	virtual const pipeline_binding_stages_collection& stage_collection() const = 0;

	/**
	*	@brief	Creates the Vulkan binding descriptor
	*/
	operator vk::vk_descriptor_set_layout_binding() const {
		stage_flag stage = stage_collection();
		return vk::vk_descriptor_set_layout_binding(vk_descriptor_type(),
													static_cast<VkShaderStageFlags>(stage),
													bind_idx(),
													count());
	}
};

}
}
