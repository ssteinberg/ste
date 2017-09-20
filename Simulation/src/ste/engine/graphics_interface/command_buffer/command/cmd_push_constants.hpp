//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_push_constant_layout.hpp>
#include <vk_pipeline_layout.hpp>

#include <stage_flag.hpp>

namespace ste {
namespace gl {

class cmd_push_constants : public command {
private:
	VkPipelineLayout layout;
	stage_flag stage;
	std::uint32_t offset;
	lib::string data;

public:
	cmd_push_constants(cmd_push_constants &&) = default;
	cmd_push_constants(const cmd_push_constants&) = default;
	cmd_push_constants &operator=(cmd_push_constants &&) = default;
	cmd_push_constants &operator=(const cmd_push_constants&) = default;

	cmd_push_constants(const vk::vk_pipeline_layout<> *layout,
					   stage_flag stage,
					   std::uint32_t offset,
					   lib::string data)
		: layout(*layout),
		  stage(stage),
		  offset(offset),
		  data(data) {}

	virtual ~cmd_push_constants() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdPushConstants(command_buffer,
						   layout,
						   static_cast<VkShaderStageFlags>(stage),
						   offset,
						   static_cast<std::uint32_t>(data.size()),
						   data.data());
	}
};

}
}
