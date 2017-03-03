//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>

namespace StE {
namespace GL {

class vk_cmd_draw : public vk_command {
private:
	std::uint32_t vertex_count;
	std::uint32_t instance_count;
	std::uint32_t first_vertex;
	std::uint32_t first_instance;

public:
	vk_cmd_draw(std::uint32_t vertex_count,
				std::uint32_t instance_count,
				std::uint32_t first_vertex = 0,
				std::uint32_t first_instance = 0)
		: vertex_count(vertex_count), instance_count(instance_count), first_vertex(first_vertex), first_instance(first_instance)
	{}
	virtual ~vk_cmd_draw() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
	}
};

}
}
