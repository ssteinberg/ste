//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_buffer_impl.hpp>

#include <vk_host_allocator.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
using vk_buffer_dense = _detail::vk_buffer_impl<false, host_allocator>;


}

}
}
