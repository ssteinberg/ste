//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <map>

namespace ste {
namespace lib {

template <typename K, typename V, class P = std::less<K>>
using map = std::map<K, V, P, allocator<std::pair<const K, V>>>;

}
}
