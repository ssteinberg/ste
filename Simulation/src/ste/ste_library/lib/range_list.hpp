//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <range_list.hpp>

namespace ste {
namespace lib {

template <typename T = std::size_t>
using range_list = range_list<T, allocator<T>>;

}
}
