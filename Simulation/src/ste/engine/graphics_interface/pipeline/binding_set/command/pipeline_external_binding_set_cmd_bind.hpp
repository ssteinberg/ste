//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <command_buffer.hpp>
#include <command_recorder.hpp>
#include <cmd_bind_descriptor_sets.hpp>

namespace ste {
namespace gl {

// Bind command
class pipeline_external_binding_set_cmd_bind : public command {
	friend class pipeline_external_binding_set;

private:
	const pipeline_external_binding_set* set;
	VkPipelineBindPoint bind_point;
	std::uint32_t base_set_index;
	const vk::vk_pipeline_layout<>* layout;

public:
	pipeline_external_binding_set_cmd_bind(const pipeline_external_binding_set* set,
	                                       VkPipelineBindPoint bind_point,
	                                       std::uint32_t base_set_index,
	                                       const vk::vk_pipeline_layout<>* layout)
		: set(set),
		  bind_point(bind_point),
		  base_set_index(base_set_index),
		  layout(layout) {}

	virtual ~pipeline_external_binding_set_cmd_bind() noexcept {}

private:
	void operator()(const command_buffer& buffer, command_recorder& recorder) && override final;
};

}
}
