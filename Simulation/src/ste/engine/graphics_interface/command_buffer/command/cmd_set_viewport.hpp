//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

#include <rect.hpp>
#include <depth_range.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_set_viewport : public command {
private:
	lib::vector<VkViewport> viewports;
	std::uint32_t first_viewport_index;

public:
	cmd_set_viewport(cmd_set_viewport &&) = default;
	cmd_set_viewport(const cmd_set_viewport&) = default;
	cmd_set_viewport &operator=(cmd_set_viewport &&) = default;
	cmd_set_viewport &operator=(const cmd_set_viewport&) = default;

	cmd_set_viewport(const lib::vector<std::pair<rect, depth_range>> &viewports,
					 std::uint32_t first_viewport_index = 0) : first_viewport_index(first_viewport_index) {
		this->viewports.reserve(viewports.size());
		for (auto &r : viewports) {
			VkViewport t;
			t.x = r.first.origin.x;
			t.y = r.first.origin.y;
			t.width = r.first.size.x;
			t.height = r.first.size.y;
			t.minDepth = r.second.min_depth;
			t.maxDepth = r.second.max_depth;

			this->viewports.push_back(t);
		}
	}

	cmd_set_viewport(const rect &viewport,
					 const depth_range &depth,
					 std::uint32_t first_viewport_index = 0)
		: cmd_set_viewport(lib::vector<std::pair<rect, depth_range>>{ std::make_pair(viewport, depth) }, first_viewport_index) {}

	virtual ~cmd_set_viewport() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdSetViewport(command_buffer,
						 first_viewport_index,
						 static_cast<std::uint32_t>(viewports.size()),
						 viewports.data());
	}
};

}
}
