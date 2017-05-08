//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command_recorder.hpp>
#include <cmd_bind_vertex_buffers.hpp>
#include <cmd_bind_index_buffer.hpp>

#include <device_buffer_base.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <buffer_view.hpp>
#include <device_image_base.hpp>

#include <draw_indirect_command_block.hpp>
#include <draw_indexed_indirect_command_block.hpp>
#include <dispatch_indirect_command_block.hpp>

#include <vector>
#include <optional.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Next, typename... Tail>
struct task_expand_vertex_buffers {
	void operator()(std::vector<std::reference_wrapper<const device_buffer_base>> &vertex_buffers,
					std::vector<std::uint64_t> &offsets,
					Next&& next,
					Tail&&... tail) {
		vertex_buffers.push_back(std::ref(std::forward<Next>(next)));
		offsets.push_back(0);

		task_expand_vertex_buffers<Tail...>()(vertex_buffers,
											  offsets,
											  std::forward<Tail>(tail)...);
	}
};
template <typename Next>
struct task_expand_vertex_buffers<Next> {
	void operator()(std::vector<std::reference_wrapper<const device_buffer_base>> &,
					std::vector<std::uint64_t> &,
					Next&&) {
	}
};

}

template <typename Interface>
struct task_interface_binder {
	void operator()(const Interface *interface,
					command_recorder &recorder) {}
};

// Interface for tasks that use vertex buffers
class task_vertex_buffers_interface {
	std::uint32_t first_binding_index{ 0 };
	std::vector<std::reference_wrapper<const device_buffer_base>> vertex_buffers;
	std::vector<std::uint64_t> offsets;

public:
	virtual ~task_vertex_buffers_interface() noexcept {}

	void set_vertex_first_binding_index(std::uint32_t first_binding_index) {
		this->first_binding_index = first_binding_index;
	}
	template <typename... VertexBuffers>
	void attach_vertex_buffers(VertexBuffers&&... vertex_buffers) {
		vertex_buffers.clear();
		offsets.clear();
		_internal::task_expand_vertex_buffers<decltype(*this), VertexBuffers...>()(vertex_buffers,
																				   offsets,
																				   std::forward<VertexBuffers>(vertex_buffers)...);
	}
	void attach_vertex_buffer(const device_buffer_base &vertex_buffer,
							  std::uint64_t offset = 0) {
		vertex_buffers = { std::ref(vertex_buffer) };
		offsets = { offset };
	}

	auto &get_vertex_first_binding_index() const { return first_binding_index; }
	auto &get_vertex_buffers() const { return vertex_buffers; }
	auto &get_vertex_offsets() const { return offsets; }
};
template <>
struct task_interface_binder<task_vertex_buffers_interface> {
	void operator()(const task_vertex_buffers_interface *interface,
					command_recorder &recorder) {
		recorder << cmd_bind_vertex_buffers(interface->get_vertex_first_binding_index(),
											interface->get_vertex_buffers(),
											interface->get_vertex_offsets());
	}
};

// Interface for tasks that use an index buffer
class task_index_buffer_interface {
	const device_buffer_base *index_buffer{ nullptr };
	std::uint64_t offset{ 0 };

public:
	virtual ~task_index_buffer_interface() noexcept {}

	void attach_index_buffer(const device_buffer_base &index_buffer,
							 std::uint64_t offset = 0) {
		this->index_buffer = &index_buffer;
		this->offset = offset;
	}

	auto &get_index_buffer() const { return *index_buffer; }
	auto get_index_offset() const { return offset; }
};
template <>
struct task_interface_binder<task_index_buffer_interface> {
	void operator()(const task_index_buffer_interface *interface,
					command_recorder &recorder) {
		recorder << cmd_bind_index_buffer(interface->get_index_buffer(),
										  interface->get_index_offset());
	}
};

// Interface for tasks that use an index buffer
template <typename IndirectBlock>
class task_indirect_buffer_interface {
	const device_buffer_base *indirect_buffer{ nullptr };
	std::uint32_t offset{ 0 };

public:
	virtual ~task_indirect_buffer_interface() noexcept {}

	void attach_indirect_buffer(const device_buffer<IndirectBlock> &index_buffer,
								std::uint32_t offset = 0) {
		this->indirect_buffer = &index_buffer;
		this->offset = offset;
	}
	void attach_indirect_buffer(const device_buffer_sparse<IndirectBlock> &index_buffer,
								std::uint32_t offset = 0) {
		this->indirect_buffer = &index_buffer;
		this->offset = offset;
	}

	auto &get_indirect_buffer() const { return *indirect_buffer; }
	auto &get_indirect_offset() const { return offset; }
};
using task_indirect_draw_buffer_interface = task_indirect_buffer_interface<draw_indirect_command_block>;
using task_indirect_indexed_draw_buffer_interface = task_indirect_buffer_interface<draw_indexed_indirect_command_block>;
using task_indirect_dispatch_buffer_interface = task_indirect_buffer_interface<dispatch_indirect_command_block>;

// Interface for a source image in transfer tasks
class task_transfer_source_image_interface {
	const device_image_base *src_image{ nullptr };
	image_layout src_image_layout{ image_layout::undefined };

public:
	virtual ~task_transfer_source_image_interface() noexcept {}

	void attach_src_image(const device_image_base &img,
						  const image_layout &layout) {
		this->src_image = &img;
		this->src_image_layout = layout;
	}

	auto &get_src_image() const { return *src_image; }
	auto &get_src_layout() const { return src_image_layout; }
};

// Interface for a destination image in transfer tasks
class task_transfer_destination_image_interface {
	const device_image_base *dst_image{ nullptr };
	image_layout dst_image_layout{ image_layout::undefined };

public:
	virtual ~task_transfer_destination_image_interface() noexcept {}

	void attach_dst_image(const device_image_base &img,
						  const image_layout &layout) {
		this->dst_image = &img;
		this->dst_image_layout = layout;
	}

	auto &get_dst_image() const { return *dst_image; }
	auto &get_dst_layout() const { return dst_image_layout; }
};

// Interface for a source buffer in transfer tasks
class task_transfer_source_buffer_interface {
	const device_buffer_base *src_buffer{ nullptr };

public:
	virtual ~task_transfer_source_buffer_interface() noexcept {}

	void attach_src_buffer(const device_buffer_base &buffer) {
		this->src_buffer = &buffer;
	}

	auto &get_src_buffer() const { return *src_buffer; }
};

// Interface for a destination buffer in transfer tasks
class task_transfer_destination_buffer_interface {
	const device_buffer_base *dst_buffer{ nullptr };

public:
	virtual ~task_transfer_destination_buffer_interface() noexcept {}

	void attach_dst_buffer(const device_buffer_base &buffer) {
		this->dst_buffer = &buffer;
	}

	auto &get_dst_buffer() const { return *dst_buffer; }
};

// Interface for a destination buffer view in transfer tasks
class task_transfer_destination_buffer_view_interface {
	optional<buffer_view> dst_buffer;

public:
	virtual ~task_transfer_destination_buffer_view_interface() noexcept {}

	void attach_dst_buffer_view(const buffer_view &buffer_view) {
		dst_buffer = buffer_view;
	}

	auto &get_dst_buffer_view() const { return dst_buffer.get(); }
};

}
}
