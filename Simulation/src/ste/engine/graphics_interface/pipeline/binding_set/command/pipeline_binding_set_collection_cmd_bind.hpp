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
class pipeline_binding_set_collection_cmd_bind : public command {
	friend class pipeline_binding_set_collection;

private:
	const pipeline_binding_set_collection* collection;
	VkPipelineBindPoint bind_point;
	std::uint32_t base_set_index;
	const vk::vk_pipeline_layout<>* layout;

public:
	pipeline_binding_set_collection_cmd_bind(const pipeline_binding_set_collection* collection,
	                                         VkPipelineBindPoint bind_point,
	                                         std::uint32_t base_set_index,
	                                         const vk::vk_pipeline_layout<>* layout)
		: collection(collection),
		  bind_point(bind_point),
		  base_set_index(base_set_index),
		  layout(layout) {}

	virtual ~pipeline_binding_set_collection_cmd_bind() noexcept {}

private:
	void operator()(const command_buffer& buffer, command_recorder& recorder) const override final;
};

}
}
