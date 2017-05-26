//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_render_pass.hpp>
#include <vk_framebuffer.hpp>

namespace ste {
namespace gl {

struct secondary_command_buffer_inheritance {
	const vk::vk_render_pass<>	*render_pass{ nullptr };
	std::uint32_t				subpass;
	const vk::vk_framebuffer<>	*framebuffer{ nullptr };
	bool						occlusion_query_enable{ false };
	VkQueryControlFlags			query_flags;
	VkQueryPipelineStatisticFlags pipeline_statistics;
};

}
}
