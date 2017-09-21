//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query_pool.hpp>

namespace ste {
namespace gl {

class cmd_reset_query_pool : public command {
private:
	VkQueryPool pool;
	std::uint32_t first;
	std::uint32_t count;

public:
	cmd_reset_query_pool(cmd_reset_query_pool &&) = default;
	cmd_reset_query_pool(const cmd_reset_query_pool&) = default;
	cmd_reset_query_pool &operator=(cmd_reset_query_pool &&) = default;
	cmd_reset_query_pool &operator=(const cmd_reset_query_pool&) = default;

	cmd_reset_query_pool(const vk::vk_query_pool<> &pool,
						 std::uint32_t first,
						 std::uint32_t count)
		: pool(pool),
		  first(first),
		  count(count) {}

	virtual ~cmd_reset_query_pool() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdResetQueryPool(command_buffer, pool, first, count);
	}
};

}
}
