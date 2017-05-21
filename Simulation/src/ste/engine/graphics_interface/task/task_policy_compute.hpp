//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <cmd_dispatch.hpp>
#include <cmd_dispatch_indirect.hpp>


namespace ste {
namespace gl {

struct task_policy_compute : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::compute_queue>;
};

// Policy for dispatch compute
template <>
struct task_policy<cmd_dispatch> : task_policy_compute {
	using interface = struct{};
};

// Policy for dispatch compute indirect
template <>
struct task_policy<cmd_dispatch_indirect> : task_policy_compute {
	using interface_types = std::tuple<task_indirect_dispatch_buffer_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_indirect_buffer(),
					   task->get_indirect_offset(),
					   std::forward<CmdArgs>(args)...);
	}
};

}
}
