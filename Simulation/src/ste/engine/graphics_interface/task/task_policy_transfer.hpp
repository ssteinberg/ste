//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <cmd_blit_image.hpp>
#include <cmd_clear_color_image.hpp>
#include <cmd_clear_depth_stencil_image.hpp>
#include <cmd_copy_image.hpp>
#include <cmd_copy_buffer.hpp>
#include <cmd_copy_buffer_to_image.hpp>
#include <cmd_copy_image_to_buffer.hpp>
#include <cmd_fill_buffer.hpp>
#include <cmd_update_buffer.hpp>

namespace ste {
namespace gl {

struct task_policy_transfer : task_policy_common {
	using pipeline_policy = task_pipeline_policy<ste_queue_type::data_transfer_queue>;
};

// Policy for blitting from image to image
template <>
struct task_policy<cmd_blit_image> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_source_image_interface, task_transfer_destination_image_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

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

// Policy for clearing color image
template <>
struct task_policy<cmd_clear_color_image> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_destination_image_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_dst_image(),
					   task->get_dst_layout(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for clearing depth/stencil image
template <>
struct task_policy<cmd_clear_depth_stencil_image> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_destination_image_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_dst_image(),
					   task->get_dst_layout(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for copying from buffer to buffer
template <>
struct task_policy<cmd_copy_buffer> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_source_buffer_interface, task_transfer_destination_buffer_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_src_buffer(),
					   task->get_dst_buffer(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for copying from buffer to image
template <>
struct task_policy<cmd_copy_buffer_to_image> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_source_buffer_interface, task_transfer_destination_image_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_src_buffer(),
					   task->get_dst_image(),
					   task->get_dst_layout(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for copying from image to image
template <>
struct task_policy<cmd_copy_image> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_source_image_interface, task_transfer_destination_image_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

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

// Policy for copying from image to buffer
template <>
struct task_policy<cmd_copy_image_to_buffer> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_source_image_interface, task_transfer_destination_buffer_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_src_image(),
					   task->get_src_layout(),
					   task->get_dst_buffer(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for clearing buffer with constant
template <>
struct task_policy<cmd_fill_buffer> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_destination_buffer_view_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_dst_buffer_view(),
					   std::forward<CmdArgs>(args)...);
	}
};

// Policy for updating buffer with data
template <>
struct task_policy<cmd_update_buffer> : task_policy_transfer {
	using interface_types = std::tuple<task_transfer_destination_buffer_view_interface>;
	using interface = inherit_from_tuple_types<interface_types>;

	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_dst_buffer_view(),
					   std::forward<CmdArgs>(args)...);
	}
};

}
}
