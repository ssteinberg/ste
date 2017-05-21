//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/small_allocator.hpp>

#include <vector>

namespace ste {
namespace lib {

template <typename T, std::size_t stack_size = 128>
using small_vector = std::vector<T, small_allocator<T, stack_size>>;

}
}
