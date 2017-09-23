//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace ste {
namespace gl {

class cmd_end_query : public command {
private:
	VkQueryPool pool;
	std::uint32_t index;

public:
	cmd_end_query(cmd_end_query&&) = default;
	cmd_end_query(const cmd_end_query&) = default;
	cmd_end_query &operator=(cmd_end_query&&) = default;
	cmd_end_query &operator=(const cmd_end_query&) = default;

	cmd_end_query(const vk::vk_query<> &query) : pool(query.get_pool()), index(query.ge_query_index()) {}
	virtual ~cmd_end_query() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdEndQuery(command_buffer,
					  pool,
					  index);
	}
};

}
}
