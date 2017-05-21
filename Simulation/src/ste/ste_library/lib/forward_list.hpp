//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <forward_list>

namespace ste {
namespace lib {

template <typename T>
using forward_list = std::forward_list<T, allocator<T>>;

}
}
