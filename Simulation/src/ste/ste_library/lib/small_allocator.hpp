//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <lib/allocator.hpp>
#include <small_allocator.hpp>

namespace ste {
namespace lib {

template <typename T, std::size_t stack_size = 128>
using small_allocator = ::ste::small_allocator<T, stack_size, allocator<T>>;

}
}
