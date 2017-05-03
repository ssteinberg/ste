//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_policy.hpp>
#include <task_pipeline_policy.hpp>
#include <task_interface.hpp>

#include <command_recorder.hpp>

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

// Policy for clearing color image
template <>
struct task_policy<cmd_clear_color_image> : task_policy_transfer {
	class interface
		: public _internal::task_transfer_destination_image_interface {
		friend struct task_policy<cmd_clear_color_image>;
		using _internal::task_transfer_destination_image_interface::get_dst_image;
		using _internal::task_transfer_destination_image_interface::get_dst_layout;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_destination_image_interface {
		friend struct task_policy<cmd_clear_depth_stencil_image>;
		using _internal::task_transfer_destination_image_interface::get_dst_image;
		using _internal::task_transfer_destination_image_interface::get_dst_layout;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_source_buffer_interface,
		public _internal::task_transfer_destination_buffer_interface {
		friend struct task_policy<cmd_copy_buffer>;
		using _internal::task_transfer_source_buffer_interface::get_src_buffer;
		using _internal::task_transfer_destination_buffer_interface::get_dst_buffer;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_source_buffer_interface,
		public _internal::task_transfer_destination_image_interface {
		friend struct task_policy<cmd_copy_buffer_to_image>;
		using _internal::task_transfer_source_buffer_interface::get_src_buffer;
		using _internal::task_transfer_destination_image_interface::get_dst_image;
		using _internal::task_transfer_destination_image_interface::get_dst_layout;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_source_image_interface,
		public _internal::task_transfer_destination_image_interface {
		friend struct task_policy<cmd_copy_image>;
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

// Policy for copying from image to buffer
template <>
struct task_policy<cmd_copy_image_to_buffer> : task_policy_transfer {
	class interface
		: public _internal::task_transfer_source_image_interface,
		public _internal::task_transfer_destination_buffer_interface {
		friend struct task_policy<cmd_copy_image_to_buffer>;
		using _internal::task_transfer_source_image_interface::get_src_image;
		using _internal::task_transfer_source_image_interface::get_src_layout;
		using _internal::task_transfer_destination_buffer_interface::get_dst_buffer;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_destination_buffer_view_interface {
		friend struct task_policy<cmd_fill_buffer>;
		using _internal::task_transfer_destination_buffer_view_interface::get_dst_buffer_view;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
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
	class interface
		: public _internal::task_transfer_destination_buffer_view_interface {
		friend struct task_policy<cmd_update_buffer>;
		using _internal::task_transfer_destination_buffer_view_interface::get_dst_buffer_view;
	public:
		virtual ~interface() {}
	};
	static void prepare(const interface *task, command_recorder &recorder) {}
	template <typename Command, typename... CmdArgs>
	static auto create_cmd(const interface *task,
						   CmdArgs&&... args) {
		return Command(task->get_dst_buffer_view(),
					   std::forward<CmdArgs>(args)...);
	}
};

}
}
