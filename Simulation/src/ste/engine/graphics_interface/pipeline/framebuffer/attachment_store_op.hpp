//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class attachment_store_op : std::uint32_t {
	store = VK_ATTACHMENT_STORE_OP_STORE,
	discard = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

}
}
