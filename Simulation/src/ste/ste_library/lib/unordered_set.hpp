//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <unordered_set>

namespace ste {
namespace lib {

template <typename T, class Hasher = std::hash<T>, class KeyEq = std::equal_to<T>>
using unordered_set = std::unordered_set<T, Hasher, KeyEq, allocator<T>>;

}
}
