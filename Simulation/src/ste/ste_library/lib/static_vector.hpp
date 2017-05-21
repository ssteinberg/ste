//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <static_vector.hpp>

namespace ste {
namespace lib {

template <typename T>
using static_vector = static_vector<T, allocator<T>>;

}
}
