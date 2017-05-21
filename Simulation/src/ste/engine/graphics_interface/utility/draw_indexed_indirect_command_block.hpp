//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std140.hpp>

namespace ste {
namespace gl {

//typedef struct draw_indexed_indirect_command {
//	uint32_t    index_count;
//	uint32_t    instance_count;
//	uint32_t    first_index;
//	int32_t     vertex_offset;
//	uint32_t    first_instance;
//};

using draw_indexed_indirect_command_std140 = std140<std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t>;

struct draw_indexed_indirect_command_block : draw_indexed_indirect_command_std140 {
	using Base = draw_indexed_indirect_command_std140;
	using Base::Base;

	uint32_t& index_count() { return Base::get<0>(); }
	uint32_t& instance_count() { return Base::get<1>(); }
	uint32_t& first_index() { return Base::get<2>(); }
	int32_t & vertex_offset() { return Base::get<3>(); }
	uint32_t& first_instance() { return Base::get<4>(); }

	const uint32_t& index_count() const { return Base::get<0>(); }
	const uint32_t& instance_count() const { return Base::get<1>(); }
	const uint32_t& first_index() const { return Base::get<2>(); }
	const int32_t & vertex_offset() const { return Base::get<3>(); }
	const uint32_t& first_instance() const { return Base::get<4>(); }
};

}
}
