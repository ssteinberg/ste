//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_push_constant_layout.hpp>
#include <vk_pipeline_layout.hpp>

namespace ste {
namespace gl {

class cmd_push_constants : public command {
private:
	const vk::vk_pipeline_layout *layout;
	VkShaderStageFlags stage;
	std::uint32_t offset;
	std::string data;

public:
	cmd_push_constants(const vk::vk_pipeline_layout *layout,
					   VkShaderStageFlags stage,
					   std::uint32_t offset,
					   std::string data)
		: layout(layout), stage(stage), offset(offset), data(data)
	{}
	virtual ~cmd_push_constants() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdPushConstants(command_buffer,
						   *layout,
						   stage,
						   offset,
						   data.size(),
						   data.data());
	}
};

}
}
