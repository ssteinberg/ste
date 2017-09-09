//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

namespace ste {
namespace gl {

//typedef struct dispatch_indirect_command {
//	uint32_t    x;
//	uint32_t    y;
//	uint32_t    z;
//};

using dispatch_indirect_command_std430_t = std430<std::uint32_t, std::uint32_t, std::uint32_t>;

template <typename BlockType>
struct dispatch_indirect_command_block_t : BlockType {
	using Base = BlockType;
	using Base::Base;

	uint32_t& x() { return Base::template get<0>(); }
	uint32_t& y() { return Base::template get<1>(); }
	uint32_t& z() { return Base::template get<2>(); }

	const uint32_t& x() const { return Base::template get<0>(); }
	const uint32_t& y() const { return Base::template get<1>(); }
	const uint32_t& z() const { return Base::template get<2>(); }
};

using dispatch_indirect_command_block = dispatch_indirect_command_block_t<dispatch_indirect_command_std430_t>;

}
}
