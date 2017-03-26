//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include "vk_render_pass.hpp"
#include "vk_framebuffer.hpp"

namespace StE {
namespace GL {

struct secondary_command_buffer_inheritance {
	const vk_render_pass	*render_pass;
	std::uint32_t			subpass;
	const vk_framebuffer	*framebuffer;
	bool					occlusion_query_enable;
	VkQueryControlFlags		query_flags;
	VkQueryPipelineStatisticFlags pipeline_statistics;
};

}
}
