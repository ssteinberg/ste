//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_pipeline_compute.hpp>
#include <vk_pipeline_graphics.hpp>

namespace StE {
namespace GL {

class vk_cmd_bind_pipeline : public vk_command {
private:
	VkPipeline pipeline;
	VkPipelineBindPoint bind_point;

public:
	vk_cmd_bind_pipeline(const vk_pipeline_compute &pipeline) : pipeline(pipeline), bind_point(VK_PIPELINE_BIND_POINT_COMPUTE)
	{}
	vk_cmd_bind_pipeline(const vk_pipeline_graphics &pipeline) : pipeline(pipeline), bind_point(VK_PIPELINE_BIND_POINT_GRAPHICS)
	{}
	virtual ~vk_cmd_bind_pipeline() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBindPipeline(command_buffer, bind_point, pipeline);
	}
};

}
}
