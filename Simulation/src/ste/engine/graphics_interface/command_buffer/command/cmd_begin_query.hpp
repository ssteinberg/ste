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
	const vk::vk_query<> &query;
	bool precise;

public:
	cmd_begin_query(const vk::vk_query<> &query,
					bool precise = false) : query(query), precise(precise) {}
	virtual ~cmd_begin_query() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdBeginQuery(command_buffer,
						query.get_pool(),
						query.ge_query_index(),
						precise ? VK_QUERY_CONTROL_PRECISE_BIT : 0);
	}
};

}
}
