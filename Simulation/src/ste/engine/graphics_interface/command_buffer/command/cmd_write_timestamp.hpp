//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace ste {
namespace gl {

class cmd_write_timestamp : public command {
private:
	const vk::vk_query<> &query;
	VkPipelineStageFlagBits stage;

public:
	cmd_write_timestamp(cmd_write_timestamp &&) = default;
	cmd_write_timestamp(const cmd_write_timestamp&) = default;
	cmd_write_timestamp &operator=(cmd_write_timestamp &&) = default;
	cmd_write_timestamp &operator=(const cmd_write_timestamp&) = default;

	cmd_write_timestamp(const vk::vk_query<> &query,
						VkPipelineStageFlagBits stage) : query(query), stage(stage) {}

	virtual ~cmd_write_timestamp() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdWriteTimestamp(command_buffer,
							stage,
							query.get_pool(),
							query.ge_query_index());
	}
};

}
}
