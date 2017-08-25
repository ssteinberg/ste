//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <unordered_map>

namespace ste {
namespace lib {

template <typename K, typename V, class Hasher = std::hash<K>, class KeyEq = std::equal_to<K>>
using unordered_multimap = std::unordered_multimap<K, V, Hasher, KeyEq, allocator<std::pair<const K, V>>>;

}
}
