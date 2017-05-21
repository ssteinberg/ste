//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <boost/container/flat_map.hpp>

namespace ste {
namespace lib {

template <typename K, typename V, class P = std::less<K>>
using flat_map = boost::container::flat_map<K, V, P, allocator<std::pair<K, V>>>;

}
}
