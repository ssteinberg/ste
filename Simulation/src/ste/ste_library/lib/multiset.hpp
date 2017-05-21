//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <set>

namespace ste {
namespace lib {

template <typename T, class P = std::less<T>>
using multiset = std::multiset<T, P, allocator<T>>;

}
}
