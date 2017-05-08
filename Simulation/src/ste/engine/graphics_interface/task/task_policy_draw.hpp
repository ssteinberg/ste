//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <cmd_draw.hpp>
#include <cmd_draw_indexed.hpp>
#include <cmd_draw_indirect.hpp>
#include <cmd_draw_indexed_indirect.hpp>

#include <inherit_from_types.hpp>

namespace ste {
namespace gl {

struct task_policy_draw : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::graphics_queue>;
};

// Policy for draw
template <>
struct task_policy<cmd_draw> : task_policy_draw {
	using interface_types = std::tuple<task_vertex_buffers_interface>;
	using interface = inherit_from_tuple_types<interface_types>;
};

// Policy for indexed draw
template <>
struct task_policy<cmd_draw_indexed> : task_policy_draw {
	using interface_types = std::tuple<task_vertex_buffers_interface, task_index_buffer_interface>;
	using interface = inherit_from_tuple_types<interface_types>;
};

// Policy for indirect draw
template <>
struct task_policy<cmd_draw_indirect> : task_policy_draw {
	using interface_types = std::tuple<task_vertex_buffers_interface, task_indirect_draw_buffer_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_indirect_buffer(),
					   task->get_indirect_offset(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for indexed indirect draw
template <>
struct task_policy<cmd_draw_indexed_indirect> : task_policy_draw {
	using interface_types = std::tuple<task_vertex_buffers_interface, task_index_buffer_interface, task_indirect_indexed_draw_buffer_interface>;
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
