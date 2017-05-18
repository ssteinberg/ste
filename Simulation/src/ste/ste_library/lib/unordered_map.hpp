//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <unordered_map>

namespace ste {
namespace lib {

template <typename K, typename V, class Hasher = std::hash<K>, class KeyEq = std::equal_to<K>>
using unordered_map = std::unordered_map<K, V, Hasher, KeyEq, allocator<std::pair<K, V>>>;

}
}
