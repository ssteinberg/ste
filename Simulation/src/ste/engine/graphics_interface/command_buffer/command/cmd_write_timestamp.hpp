//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace ste {
namespace gl {

class cmd_write_timestamp : public command {
private:
	VkQueryPool pool;
	std::uint32_t index;
	gl::pipeline_stage stage;

public:
	cmd_write_timestamp(cmd_write_timestamp &&) = default;
	cmd_write_timestamp(const cmd_write_timestamp&) = default;
	cmd_write_timestamp &operator=(cmd_write_timestamp &&) = default;
	cmd_write_timestamp &operator=(const cmd_write_timestamp&) = default;

	cmd_write_timestamp(const vk::vk_query<> &query,
						gl::pipeline_stage stage)
		: pool(query.get_pool()), index(query.ge_query_index()), stage(stage) {}

	virtual ~cmd_write_timestamp() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdWriteTimestamp(command_buffer,
							static_cast<VkPipelineStageFlagBits>(stage),
							pool,
							index);
	}
};

}
}
