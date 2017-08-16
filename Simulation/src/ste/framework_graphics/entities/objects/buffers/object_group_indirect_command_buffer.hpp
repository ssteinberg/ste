// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <vector.hpp>
#include <draw_indexed_indirect_command_block.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace graphics {

class object_group_indirect_command_buffer : public allow_type_decay<object_group_indirect_command_buffer, gl::vector<gl::draw_indexed_indirect_command_block>> {
private:
	using indirect_draw_buffer_type = gl::vector<gl::draw_indexed_indirect_command_block>;

private:
	indirect_draw_buffer_type idb;

public:
	object_group_indirect_command_buffer(const ste_context &ctx,
										 const gl::buffer_usage &usage = static_cast<gl::buffer_usage>(0))
		: idb(ctx, gl::buffer_usage::indirect_buffer | usage)
	{}

	auto &get() { return idb; }
	auto &get() const { return idb; }
};

}
}
