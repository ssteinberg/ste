//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

#include <rect.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_set_scissor : public command {
private:
	lib::vector<VkRect2D> scissors;
	std::uint32_t first_scissor_index;

public:
	cmd_set_scissor(cmd_set_scissor &&) = default;
	cmd_set_scissor(const cmd_set_scissor&) = default;
	cmd_set_scissor &operator=(cmd_set_scissor &&) = default;
	cmd_set_scissor &operator=(const cmd_set_scissor&) = default;

	cmd_set_scissor(const lib::vector<i32rect> &scissors,
					std::uint32_t first_scissor_index = 0) : first_scissor_index(first_scissor_index) {
		this->scissors.reserve(scissors.size());
		for (auto &r : scissors) {
			VkRect2D t;
			t.offset.x = r.origin.x;
			t.offset.y = r.origin.y;
			t.extent.width = r.size.x;
			t.extent.height = r.size.y;

			this->scissors.push_back(t);
		}
	}

	cmd_set_scissor(const i32rect &scissor,
					std::uint32_t first_scissor_index = 0)
		: cmd_set_scissor(lib::vector<i32rect>{ scissor }, first_scissor_index) {}

	virtual ~cmd_set_scissor() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdSetScissor(command_buffer,
						first_scissor_index,
						static_cast<std::uint32_t>(scissors.size()),
						scissors.data());
	}
};

}
}
