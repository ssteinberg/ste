//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace StE {
namespace GL {

class vk_cmd_write_timestamp : public vk_command {
private:
	const vk_query &query;
	VkPipelineStageFlagBits stage;

public:
	vk_cmd_write_timestamp(const vk_query &query,
						   VkPipelineStageFlagBits stage) : query(query), stage(stage) {}
	virtual ~vk_cmd_write_timestamp() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdWriteTimestamp(command_buffer,
							stage,
							query.get_pool(),
							query.ge_query_index());
	}
};

}
}
