//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

namespace ste {
namespace gl {

//typedef struct draw_indirect_command {
//	uint32_t    vertex_count;
//	uint32_t    instance_count;
//	uint32_t    first_vertex;
//	uint32_t    first_instance;
//};

using draw_indirect_command_std430_t = std430<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>;

template <typename BlockType>
struct draw_indirect_command_block_t : BlockType {
	using Base = BlockType;
	using Base::Base;

	uint32_t& vertex_count() { return Base::template get<0>(); }
	uint32_t& instance_count() { return Base::template get<1>(); }
	uint32_t& first_vertex() { return Base::template get<2>(); }
	uint32_t& first_instance() { return Base::template get<3>(); }

	const uint32_t& vertex_count() const { return Base::template get<0>(); }
	const uint32_t& instance_count() const { return Base::template get<1>(); }
	const uint32_t& first_vertex() const { return Base::template get<2>(); }
	const uint32_t& first_instance() const { return Base::template get<3>(); }
};

using draw_indirect_command_block = draw_indirect_command_block_t<draw_indirect_command_std430_t>;

}
}
