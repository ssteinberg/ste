//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>

namespace StE {
namespace GL {

class vk_cmd_draw_indexed : public vk_command {
private:
	std::uint32_t index_count;
	std::uint32_t instance_count;
	std::uint32_t first_index;
	std::uint32_t vertex_offset;
	std::uint32_t first_instance;

public:
	vk_cmd_draw_indexed(std::uint32_t index_count,
						std::uint32_t instance_count,
						std::uint32_t first_index = 0,
						std::uint32_t vertex_offset = 0,
						std::uint32_t first_instance = 0)
		: index_count(index_count),
		instance_count(instance_count),
		first_index(first_index),
		vertex_offset(vertex_offset),
		first_instance(first_instance)
	{}
	virtual ~vk_cmd_draw_indexed() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDrawIndexed(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
	}
};

}
}
