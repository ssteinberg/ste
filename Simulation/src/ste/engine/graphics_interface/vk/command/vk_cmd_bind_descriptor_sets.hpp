//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_descriptor_set.hpp>

#include <vector>
#include <functional>

namespace StE {
namespace GL {

class vk_cmd_bind_descriptor_sets : public vk_command {
private:
	VkPipelineBindPoint bind_point;
	const vk_pipeline_layout &pipeline_layout;
	std::uint32_t first_set_bind_index;
	std::vector<VkDescriptorSet> sets;
	std::vector<std::uint32_t> dynamic_offsets;

protected:
	vk_cmd_bind_descriptor_sets(VkPipelineBindPoint bind_point,
								const vk_pipeline_layout &pipeline_layout,
								std::uint32_t first_set_bind_index,
								const std::vector<std::reference_wrapper<vk_descriptor_set>> &sets,
								const std::vector<std::uint32_t> &dynamic_offsets)
		: bind_point(bind_point),
		pipeline_layout(pipeline_layout),
		first_set_bind_index(first_set_bind_index),
		dynamic_offsets(dynamic_offsets) 
	{
		this->sets.resize(sets.size());
		for (int i = 0; i < sets.size(); ++i)
			this->sets[i] = (sets.begin() + i)->get();
	}

public:
	virtual ~vk_cmd_bind_descriptor_sets() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBindDescriptorSets(command_buffer, 
								bind_point, 
								pipeline_layout, 
								first_set_bind_index, 
								sets.size(),
								sets.data(),
								dynamic_offsets.size(),
								dynamic_offsets.data());
	}
};

class vk_cmd_bind_descriptor_sets_compute : public vk_cmd_bind_descriptor_sets {
public:
	vk_cmd_bind_descriptor_sets_compute(const vk_pipeline_layout &pipeline_layout,
										std::uint32_t first_set_bind_index,
										const std::vector<std::reference_wrapper<vk_descriptor_set>> &sets,
										const std::vector<std::uint32_t> &dynamic_offsets)
		: vk_cmd_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE,
									  pipeline_layout,
									  first_set_bind_index,
									  sets,
									  dynamic_offsets)
	{}
};

class vk_cmd_bind_descriptor_sets_graphics : public vk_cmd_bind_descriptor_sets {
public:
	vk_cmd_bind_descriptor_sets_graphics(const vk_pipeline_layout &pipeline_layout,
										 std::uint32_t first_set_bind_index,
										 const std::vector<std::reference_wrapper<vk_descriptor_set>> &sets,
										 const std::vector<std::uint32_t> &dynamic_offsets)
		: vk_cmd_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS,
									  pipeline_layout,
									  first_set_bind_index,
									  sets,
									  dynamic_offsets)
	{}
};

}
}
