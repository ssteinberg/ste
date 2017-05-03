//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <command_recorder.hpp>
#include <cmd_dispatch.hpp>
#include <cmd_dispatch_indirect.hpp>

#include <dispatch_indirect_command_block.hpp>

namespace ste {
namespace gl {

struct task_policy_compute : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::compute_queue>;
};

// Policy for dispatch compute
template <>
struct task_policy<cmd_dispatch> : task_policy_compute {
	class interface {
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
};

// Policy for dispatch compute indirect
template <>
struct task_policy<cmd_dispatch_indirect> : task_policy_compute {
	class interface : public _internal::task_indirect_buffer_interface<dispatch_indirect_command_block> {
		friend struct task_policy<cmd_dispatch_indirect>;
		using _internal::task_indirect_buffer_interface<dispatch_indirect_command_block>::get_buffer;
		using _internal::task_indirect_buffer_interface<dispatch_indirect_command_block>::get_offset;
	public:
		virtual ~interface() {}
	};

	static void prepare(const interface *task, command_recorder &recorder) {}
	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_buffer(),
					   task->get_offset(),
					   std::forward<CmdArgs>(args)...);
	}
};

}
}
