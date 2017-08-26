//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_descriptor_set.hpp>

#include <lib/vector.hpp>
#include <functional>

namespace ste {
namespace gl {

class cmd_bind_descriptor_sets : public command {
private:
	VkPipelineBindPoint bind_point;
	std::reference_wrapper<const vk::vk_pipeline_layout<>> pipeline_layout;
	std::uint32_t first_set_bind_index;
	lib::vector<VkDescriptorSet> sets;
	lib::vector<std::uint32_t> dynamic_offsets;

public:
	cmd_bind_descriptor_sets(VkPipelineBindPoint bind_point,
							 const vk::vk_pipeline_layout<> &pipeline_layout,
							 std::uint32_t first_set_bind_index,
							 const lib::vector<const vk::vk_descriptor_set<>*> &sets,
							 const lib::vector<std::uint32_t> &dynamic_offsets = {})
		: bind_point(bind_point),
		pipeline_layout(pipeline_layout),
		first_set_bind_index(first_set_bind_index),
		dynamic_offsets(dynamic_offsets)
	{
		this->sets.reserve(sets.size());
		for (auto &s : sets)
			this->sets.push_back(*s);
	}
	virtual ~cmd_bind_descriptor_sets() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		VkPipelineLayout handle = pipeline_layout.get();
		vkCmdBindDescriptorSets(command_buffer,
								bind_point,
								handle,
								first_set_bind_index,
								static_cast<std::uint32_t>(sets.size()),
								sets.data(),
								static_cast<std::uint32_t>(dynamic_offsets.size()),
								dynamic_offsets.data());
	}
};

class cmd_bind_descriptor_sets_compute : public cmd_bind_descriptor_sets {
public:
	cmd_bind_descriptor_sets_compute(const vk::vk_pipeline_layout<> &pipeline_layout,
									 std::uint32_t first_set_bind_index,
									 const lib::vector<const vk::vk_descriptor_set<>*> &sets,
									 const lib::vector<std::uint32_t> &dynamic_offsets = {})
		: cmd_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE,
								   pipeline_layout,
								   first_set_bind_index,
								   sets,
								   dynamic_offsets)
	{}
};

class cmd_bind_descriptor_sets_graphics : public cmd_bind_descriptor_sets {
public:
	cmd_bind_descriptor_sets_graphics(const vk::vk_pipeline_layout<> &pipeline_layout,
									  std::uint32_t first_set_bind_index,
									  const lib::vector<const vk::vk_descriptor_set<>*> &sets,
									  const lib::vector<std::uint32_t> &dynamic_offsets = {})
		: cmd_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS,
								   pipeline_layout,
								   first_set_bind_index,
								   sets,
								   dynamic_offsets)
	{}
};

}
}
