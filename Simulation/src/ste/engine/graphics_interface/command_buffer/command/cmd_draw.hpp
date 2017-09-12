//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

namespace ste {
namespace gl {

class cmd_draw : public command {
private:
	std::uint32_t vertex_count;
	std::uint32_t instance_count;
	std::uint32_t first_vertex;
	std::uint32_t first_instance;

public:
	cmd_draw(cmd_draw &&) = default;
	cmd_draw(const cmd_draw&) = default;
	cmd_draw &operator=(cmd_draw &&) = default;
	cmd_draw &operator=(const cmd_draw&) = default;

	cmd_draw(std::uint32_t vertex_count,
			 std::uint32_t instance_count,
			 std::uint32_t first_vertex = 0,
			 std::uint32_t first_instance = 0)
		: vertex_count(vertex_count), instance_count(instance_count), first_vertex(first_vertex), first_instance(first_instance) {}

	virtual ~cmd_draw() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
	}
};

}
}
