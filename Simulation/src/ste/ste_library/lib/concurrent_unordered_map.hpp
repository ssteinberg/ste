//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <concurrency/concurrent_unordered_map.hpp>

namespace ste {
namespace lib {

template <typename K, typename V, int cache_line = 64>
using concurrent_unordered_map = ::ste::concurrent_unordered_map<K, V, allocator<V>, allocator<V>, cache_line>;

}
}
