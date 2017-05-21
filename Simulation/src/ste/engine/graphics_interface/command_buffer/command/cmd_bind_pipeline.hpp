//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_pipeline_compute.hpp>
#include <vk_pipeline_graphics.hpp>

namespace ste {
namespace gl {

class cmd_bind_pipeline : public command {
private:
	VkPipeline pipeline;
	VkPipelineBindPoint bind_point;

public:
	cmd_bind_pipeline(const vk::vk_pipeline_compute &pipeline) : pipeline(pipeline), bind_point(VK_PIPELINE_BIND_POINT_COMPUTE)
	{}
	cmd_bind_pipeline(const vk::vk_pipeline_graphics &pipeline) : pipeline(pipeline), bind_point(VK_PIPELINE_BIND_POINT_GRAPHICS)
	{}
	virtual ~cmd_bind_pipeline() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdBindPipeline(command_buffer, bind_point, pipeline);
	}
};

}
}
