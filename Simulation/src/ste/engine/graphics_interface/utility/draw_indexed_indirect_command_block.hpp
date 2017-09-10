//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

namespace ste {
namespace gl {

//typedef struct draw_indexed_indirect_command {
//	uint32_t    index_count;
//	uint32_t    instance_count;
//	uint32_t    first_index;
//	int32_t     vertex_offset;
//	uint32_t    first_instance;
//};

using draw_indexed_indirect_command_std430_t = std430<std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t>;

template <typename BlockType>
struct draw_indexed_indirect_command_block_t : BlockType {
	using Base = BlockType;
	using Base::Base;

	uint32_t& index_count() { return Base::template get<0>(); }
	uint32_t& instance_count() { return Base::template get<1>(); }
	uint32_t& first_index() { return Base::template get<2>(); }
	int32_t & vertex_offset() { return Base::template get<3>(); }
	uint32_t& first_instance() { return Base::template get<4>(); }

	const uint32_t& index_count() const { return Base::template get<0>(); }
	const uint32_t& instance_count() const { return Base::template get<1>(); }
	const uint32_t& first_index() const { return Base::template get<2>(); }
	const int32_t & vertex_offset() const { return Base::template get<3>(); }
	const uint32_t& first_instance() const { return Base::template get<4>(); }
};

using draw_indexed_indirect_command_block = draw_indexed_indirect_command_block_t<draw_indexed_indirect_command_std430_t>;

}
}
