//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class attachment_load_op : std::uint32_t {
	load = VK_ATTACHMENT_LOAD_OP_LOAD,
	clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
	undefined = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

}
}
