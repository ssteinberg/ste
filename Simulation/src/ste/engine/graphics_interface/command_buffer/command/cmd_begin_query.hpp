//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace ste {
namespace gl {

class cmd_begin_query : public command {
private:
	VkQueryPool pool;
	std::uint32_t index;
	bool precise;

public:
	cmd_begin_query(cmd_begin_query &&) = default;
	cmd_begin_query(const cmd_begin_query &) = default;
	cmd_begin_query &operator=(cmd_begin_query &&) = default;
	cmd_begin_query &operator=(const cmd_begin_query &) = default;

	cmd_begin_query(const vk::vk_query<> &query,
					bool precise = false)
		: pool(query.get_pool()), index(query.ge_query_index()), precise(precise) {}

	virtual ~cmd_begin_query() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdBeginQuery(command_buffer,
						pool,
						index,
						precise ? VK_QUERY_CONTROL_PRECISE_BIT : 0);
	}
};

}
}
