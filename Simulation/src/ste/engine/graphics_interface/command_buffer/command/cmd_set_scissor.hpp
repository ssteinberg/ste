//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

#include <rect.hpp>
#include <vector>

namespace ste {
namespace gl {

class cmd_set_scissor : public command {
private:
	std::vector<VkRect2D> scissors;
	std::uint32_t first_scissor_index;

public:
	cmd_set_scissor(const std::vector<i32rect> &scissors,
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
		: cmd_set_scissor(std::vector<i32rect>{ scissor }, first_scissor_index)
	{}
	virtual ~cmd_set_scissor() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdSetScissor(command_buffer, 
						first_scissor_index, 
						scissors.size(),
						scissors.data());
	}
};

}
}
