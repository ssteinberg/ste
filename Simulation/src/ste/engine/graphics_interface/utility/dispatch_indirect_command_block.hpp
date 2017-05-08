//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std140.hpp>

namespace ste {
namespace gl {

//typedef struct dispatch_indirect_command {
//	uint32_t    x;
//	uint32_t    y;
//	uint32_t    z;
//};

using dispatch_indirect_command_std140 = std140<std::uint32_t, std::uint32_t, std::uint32_t>;

struct dispatch_indirect_command_block : dispatch_indirect_command_std140 {
	using Base = dispatch_indirect_command_std140;
	using Base::Base;

	uint32_t& x() { return Base::get<0>(); }
	uint32_t& y() { return Base::get<1>(); }
	uint32_t& z() { return Base::get<2>(); }

	const uint32_t& x() const { return Base::get<0>(); }
	const uint32_t& y() const { return Base::get<1>(); }
	const uint32_t& z() const { return Base::get<2>(); }
};

}
}
