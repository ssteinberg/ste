//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query_pool.hpp>
#include <device_buffer_base.hpp>

namespace ste {
namespace gl {

class cmd_copy_query_pool_results : public command {
private:
	VkQueryPool pool;
	VkBuffer buffer;
	std::uint32_t first;
	std::uint32_t count;
	std::uint64_t offset;
	std::uint64_t stride;
	VkQueryResultFlags flags;

public:
	cmd_copy_query_pool_results(cmd_copy_query_pool_results &&) = default;
	cmd_copy_query_pool_results(const cmd_copy_query_pool_results&) = default;
	cmd_copy_query_pool_results &operator=(cmd_copy_query_pool_results &&) = default;
	cmd_copy_query_pool_results &operator=(const cmd_copy_query_pool_results&) = default;

	cmd_copy_query_pool_results(const vk::vk_query_pool<> &pool,
								const device_buffer_base &buffer,
								std::uint32_t first_query,
								std::uint32_t queries_count,
								std::uint64_t buffer_offset,
								std::uint64_t stride,
								VkQueryResultFlags flags)
		: pool(pool),
		  buffer(buffer.get_buffer_handle()),
		  first(first_query),
		  count(queries_count),
		  offset(buffer_offset * buffer.get_element_size_bytes()),
		  stride(stride * buffer.get_element_size_bytes()),
		  flags(flags) {}

	virtual ~cmd_copy_query_pool_results() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdCopyQueryPoolResults(command_buffer,
								  pool,
								  first,
								  count,
								  buffer,
								  offset,
								  stride,
								  flags);
	}
};

}
}
