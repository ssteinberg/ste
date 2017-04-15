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
	 *	@brief	Creates the Vulkan binding descriptor
	 */
	operator vk_descriptor_set_layout_binding() const {
		std::uint32_t count = binding->variable->size();
		return vk_descriptor_set_layout_binding(*binding,
												stages,
												binding->bind_idx,
												count);
	}
};

}
}
