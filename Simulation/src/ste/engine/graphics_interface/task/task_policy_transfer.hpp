//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <command_recorder.hpp>

namespace ste {
namespace gl {

struct task_policy_transfer : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::data_transfer_queue>;
};

template <>
struct task_policy<cmd_blit_image> : task_policy_transfer {
	class interface
		: public _internal::task_transfer_source_image_interface, 
		public _internal::task_transfer_destination_image_interface {
		friend struct task_policy<cmd_blit_image>;
		using _internal::task_transfer_source_image_interface::get_src_image;
		using _internal::task_transfer_source_image_interface::get_src_layout;
		using _internal::task_transfer_destination_image_interface::get_dst_image;
		using _internal::task_transfer_destination_image_interface::get_dst_layout;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_src_image(),
					   task->get_src_layout(),
					   task->get_dst_image(),
					   task->get_dst_layout(),
					   std::forward<CmdArgs>(args)...);
	}
};

}
}
