//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_resource.hpp>
#include <task_foreach_interface.hpp>

namespace ste {
namespace gl {

template <typename Interface>
struct task_interface_extract_consumed_resources {};

template <>
struct task_interface_extract_consumed_resources<task_vertex_buffers_interface> {
	auto operator()(const task_vertex_buffers_interface *interface) {
		std::vector<task_resource> ret;
		for (auto &v : interface->get_vertex_buffers()) {
			task_resource res;
			res.type = task_resource_type::buffer;
			res.handle = v.get().get_buffer_handle();
			res.access = access_flags::vertex_attribute_read;
			res.stage = pipeline_stage::vertex_shader;

			ret.push_back(res);
		}

		return ret;
	}
};
template <>
struct task_interface_extract_consumed_resources<task_index_buffer_interface> {
	auto operator()(const task_index_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_index_buffer().get_buffer_handle();
		res.access = access_flags::vertex_attribute_read;
		res.stage = pipeline_stage::vertex_shader;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_indirect_draw_buffer_interface> {
	auto operator()(const task_indirect_draw_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_indirect_buffer().get_buffer_handle();
		res.access = access_flags::indirect_command_read;
		res.stage = pipeline_stage::vertex_shader;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_indirect_indexed_draw_buffer_interface> {
	auto operator()(const task_indirect_indexed_draw_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_indirect_buffer().get_buffer_handle();
		res.access = access_flags::indirect_command_read;
		res.stage = pipeline_stage::vertex_shader;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_indirect_dispatch_buffer_interface> {
	auto operator()(const task_indirect_dispatch_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_indirect_buffer().get_buffer_handle();
		res.access = access_flags::indirect_command_read;
		res.stage = pipeline_stage::vertex_shader;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_transfer_source_image_interface> {
	auto operator()(const task_transfer_source_image_interface *interface) {
		task_resource res;
		res.type = task_resource_type::image;
		res.handle = interface->get_src_image().get_image_handle();
		res.layout = interface->get_src_layout();
		res.access = access_flags::transfer_read;
		res.stage = pipeline_stage::transfer;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_transfer_destination_image_interface> {
	auto operator()(const task_transfer_destination_image_interface *interface) {
		task_resource res;
		res.type = task_resource_type::image;
		res.handle = interface->get_dst_image().get_image_handle();
		res.layout = interface->get_dst_layout();
		res.access = access_flags::transfer_write;
		res.stage = pipeline_stage::transfer;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_transfer_source_buffer_interface> {
	auto operator()(const task_transfer_source_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_src_buffer().get_buffer_handle();
		res.access = access_flags::transfer_read;
		res.stage = pipeline_stage::transfer;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_transfer_destination_buffer_interface> {
	auto operator()(const task_transfer_destination_buffer_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_dst_buffer().get_buffer_handle();
		res.access = access_flags::transfer_write;
		res.stage = pipeline_stage::transfer;

		return std::vector<task_resource>{ res };
	}
};
template <>
struct task_interface_extract_consumed_resources<task_transfer_destination_buffer_view_interface> {
	auto operator()(const task_transfer_destination_buffer_view_interface *interface) {
		task_resource res;
		res.type = task_resource_type::buffer;
		res.handle = interface->get_dst_buffer_view().get().get_buffer_handle();
		res.access = access_flags::transfer_write;
		res.stage = pipeline_stage::transfer;

		return std::vector<task_resource>{ res };
	}
};

}
}
