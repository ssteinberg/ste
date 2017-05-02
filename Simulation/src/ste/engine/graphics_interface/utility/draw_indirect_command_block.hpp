//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std140.hpp>

namespace ste {
namespace gl {

//typedef struct draw_indirect_command {
//	uint32_t    vertex_count;
//	uint32_t    instance_count;
//	uint32_t    first_vertex;
//	uint32_t    first_instance;
//};

using draw_indirect_command_std140 = std140<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>;

struct draw_indirect_command_block : draw_indirect_command_std140 {
	using Base = draw_indirect_command_std140;
	using Base::Base;

	uint32_t& vertex_count() { return Base::get<0>(); }
	uint32_t& instance_count() { return Base::get<1>(); }
	uint32_t& first_vertex() { return Base::get<2>(); }
	uint32_t& first_instance() { return Base::get<3>(); }

	const uint32_t& vertex_count() const { return Base::get<0>(); }
	const uint32_t& instance_count() const { return Base::get<1>(); }
	const uint32_t& first_vertex() const { return Base::get<2>(); }
	const uint32_t& first_instance() const { return Base::get<3>(); }
};

}
}
