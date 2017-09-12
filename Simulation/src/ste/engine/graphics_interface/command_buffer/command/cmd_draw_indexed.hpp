//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

namespace ste {
namespace gl {

class cmd_draw_indexed : public command {
private:
	std::uint32_t index_count;
	std::uint32_t instance_count;
	std::uint32_t first_index;
	std::int32_t vertex_offset;
	std::uint32_t first_instance;

public:
	cmd_draw_indexed(cmd_draw_indexed &&) = default;
	cmd_draw_indexed(const cmd_draw_indexed&) = default;
	cmd_draw_indexed &operator=(cmd_draw_indexed &&) = default;
	cmd_draw_indexed &operator=(const cmd_draw_indexed&) = default;

	cmd_draw_indexed(std::uint32_t index_count,
					 std::uint32_t instance_count,
					 std::uint32_t first_index = 0,
					 std::int32_t vertex_offset = 0,
					 std::uint32_t first_instance = 0)
		: index_count(index_count),
		  instance_count(instance_count),
		  first_index(first_index),
		  vertex_offset(vertex_offset),
		  first_instance(first_instance) {}

	virtual ~cmd_draw_indexed() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDrawIndexed(command_buffer,
						 index_count,
						 instance_count,
						 first_index,
						 vertex_offset,
						 first_instance);
	}
};

}
}
