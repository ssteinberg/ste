//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_query_pool.hpp>

namespace StE {
namespace GL {

class vk_cmd_reset_query_pool : public vk_command {
private:
	const vk_query_pool &pool;
	std::uint32_t first;
	std::uint32_t count;

public:
	vk_cmd_reset_query_pool(const vk_query_pool &pool,
							std::uint32_t first,
							std::uint32_t count) : pool(pool), first(first), count(count) {}
	virtual ~vk_cmd_reset_query_pool() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdResetQueryPool(command_buffer, pool, first, count);
	}
};

}
}
