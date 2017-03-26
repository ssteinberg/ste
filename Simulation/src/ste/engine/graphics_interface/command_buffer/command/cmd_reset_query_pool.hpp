//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query_pool.hpp>

namespace StE {
namespace GL {

class cmd_reset_query_pool : public command {
private:
	const vk_query_pool &pool;
	std::uint32_t first;
	std::uint32_t count;

public:
	cmd_reset_query_pool(const vk_query_pool &pool,
							std::uint32_t first,
							std::uint32_t count) : pool(pool), first(first), count(count) {}
	virtual ~cmd_reset_query_pool() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdResetQueryPool(command_buffer, pool, first, count);
	}
};

}
}
