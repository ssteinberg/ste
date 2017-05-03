//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <command_recorder.hpp>
#include <cmd_draw.hpp>
#include <cmd_draw_indexed.hpp>
#include <cmd_draw_indirect.hpp>
#include <cmd_draw_indexed_indirect.hpp>

#include <draw_indirect_command_block.hpp>
#include <draw_indexed_indirect_command_block.hpp>

namespace ste {
namespace gl {

struct task_policy_draw : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::graphics_queue>;
};

// Policy for draw
template <>
struct task_policy<cmd_draw> : task_policy_draw {
	class interface : public _internal::task_vertex_buffers_interface {
		friend struct task_policy<cmd_draw>;
		void prepare(command_recorder &recorder) const {
			_internal::task_vertex_buffers_interface::bind(recorder);
		}
	public:
		virtual ~interface() {}
	};

	static void prepare(const interface *task, command_recorder &recorder) {
		task->prepare(recorder);
	}
};

// Policy for indexed draw
template <>
struct task_policy<cmd_draw_indexed> : task_policy_draw {
	class interface : public _internal::task_vertex_buffers_interface, public _internal::task_index_buffer_interface {
		friend struct task_policy<cmd_draw_indexed>;
		void prepare(command_recorder &recorder) const {
			_internal::task_vertex_buffers_interface::bind(recorder);
			_internal::task_index_buffer_interface::bind(recorder);
		}
	public:
		virtual ~interface() {}
	};

	static void prepare(const interface *task, command_recorder &recorder) {
		task->prepare(recorder);
	}
};

// Policy for indirect draw
template <>
struct task_policy<cmd_draw_indirect> : task_policy_draw {
	class interface 
		: public _internal::task_vertex_buffers_interface, 
		public _internal::task_indirect_buffer_interface<draw_indirect_command_block>
	{
		friend struct task_policy<cmd_draw_indirect>;
		void prepare(command_recorder &recorder) const {
			_internal::task_vertex_buffers_interface::bind(recorder);
		}
		using _internal::task_indirect_buffer_interface<draw_indirect_command_block>::get_buffer;
		using _internal::task_indirect_buffer_interface<draw_indirect_command_block>::get_offset;
	public:
		virtual ~interface() {}
	};

	static void prepare(const interface *task, command_recorder &recorder) {
		task->prepare(recorder);
	}
	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_buffer(),
					   task->get_offset(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for indexed indirect draw
template <>
struct task_policy<cmd_draw_indexed_indirect> : task_policy_draw {
	class interface 
		: public _internal::task_vertex_buffers_interface, 
		public _internal::task_index_buffer_interface,
		public _internal::task_indirect_buffer_interface<draw_indexed_indirect_command_block> 
	{
		friend struct task_policy<cmd_draw_indexed_indirect>;
		void prepare(command_recorder &recorder) const {
			_internal::task_vertex_buffers_interface::bind(recorder);
			_internal::task_index_buffer_interface::bind(recorder);
		}
		using _internal::task_indirect_buffer_interface<draw_indexed_indirect_command_block>::get_buffer;
		using _internal::task_indirect_buffer_interface<draw_indexed_indirect_command_block>::get_offset;
	public:
		virtual ~interface() {}
	};

	static void prepare(const interface *task, command_recorder &recorder) {
		task->prepare(recorder);
	}
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
