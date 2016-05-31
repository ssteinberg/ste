// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "buffer_usage.hpp"
#include "IndirectDrawBufferObject.hpp"

namespace StE {
namespace Graphics {

class object_group_indirect_command_buffer {
private:
	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);

	static constexpr int pages = 8192;
 	using indirect_draw_buffer_type = Core::IndirectDrawBuffer<Core::IndirectMultiDrawElementsCommand, usage>;

private:
	indirect_draw_buffer_type idb;

public:
	object_group_indirect_command_buffer() : idb(pages * std::max<std::size_t>(65536, indirect_draw_buffer_type::page_size()) / sizeof(Core::IndirectMultiDrawElementsCommand)) {}

	auto &buffer() { return idb; }
	auto &buffer() const { return idb; }

	auto ssbo() const { return Core::buffer_object_cast<Core::ShaderStorageBuffer<Core::IndirectMultiDrawElementsCommand, usage>>(buffer()); }
};

}
}
