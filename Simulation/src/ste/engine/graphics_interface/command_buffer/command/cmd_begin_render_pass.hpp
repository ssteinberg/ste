//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_framebuffer.hpp>
#include <vk_render_pass.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_begin_render_pass : public command {
private:
	VkFramebuffer framebuffer;
	VkRenderPass render_pass;
	VkRect2D render_area;
	lib::vector<VkClearValue> clear_values;

public:
	cmd_begin_render_pass(cmd_begin_render_pass &&) = default;
	cmd_begin_render_pass(const cmd_begin_render_pass&) = default;
	cmd_begin_render_pass &operator=(cmd_begin_render_pass &&) = default;
	cmd_begin_render_pass &operator=(const cmd_begin_render_pass&) = default;

	cmd_begin_render_pass(const vk::vk_framebuffer<> &framebuffer,
						  const vk::vk_render_pass<> &render_pass,
						  glm::i32vec2 render_area_offset,
						  glm::u32vec2 render_area_size,
						  const lib::vector<VkClearValue> &clear_values)
		: framebuffer(framebuffer),
		  render_pass(render_pass),
		  clear_values(clear_values) {
		render_area.offset.x = render_area_offset.x;
		render_area.offset.y = render_area_offset.y;
		render_area.extent.width = render_area_size.x;
		render_area.extent.height = render_area_size.y;
	}

	virtual ~cmd_begin_render_pass() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = render_pass;
		info.renderArea = render_area;
		info.framebuffer = framebuffer;
		info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
		info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}
};

}
}
