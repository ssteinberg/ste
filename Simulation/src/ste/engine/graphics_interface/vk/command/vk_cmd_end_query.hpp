//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_query.hpp>
#include <vk_query_pool.hpp>

namespace StE {
namespace GL {

class vk_cmd_end_query : public vk_command {
private:
	const vk_query &query;

public:
	vk_cmd_end_query(const vk_query &query) : query(query) {}
	virtual ~vk_cmd_end_query() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdEndQuery(command_buffer,
					  query.get_pool(),
					  query.ge_query_index());
	}
};

}
}
