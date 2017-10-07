//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <concurrency/concurrent_unordered_map.hpp>

#include <new>

namespace ste {
namespace lib {

template <typename K, typename V, int cache_line = std::hardware_destructive_interference_size>
using concurrent_unordered_map = 
::ste::concurrent_unordered_map<
	K, 
	V, 
	cache_line, 
	allocator<V, cache_line>, 
	allocator<V>
>;

}
}
