//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_query_pool.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class vk_cmd_copy_query_pool_results : public vk_command {
private:
	const vk_query_pool &pool;
	VkBuffer buffer;
	std::uint32_t first;
	std::uint32_t count;
	std::uint64_t offset;
	std::uint64_t stride;
	VkQueryResultFlags flags;

public:
	vk_cmd_copy_query_pool_results(const vk_query_pool &pool,
								   const vk_buffer_base &buffer,
								   std::uint32_t first_query,
								   std::uint32_t queries_count,
								   std::uint64_t buffer_offset,
								   std::uint64_t stride,
								   VkQueryResultFlags flags)
		: pool(pool),
		buffer(buffer),
		first(first_query),
		count(queries_count),
		offset(buffer_offset * buffer.get_element_size_bytes()),
		stride(stride * buffer.get_element_size_bytes()),
		flags(flags)
	{}
	virtual ~vk_cmd_copy_query_pool_results() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
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
