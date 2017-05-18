//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <vector>

namespace ste {
namespace lib {

template <typename T>
using vector = std::vector<T, allocator<T>>;

}
}
