//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <boost/container/flat_set.hpp>

namespace ste {
namespace lib {

template <typename T, class P = std::less<T>>
using flat_set = boost::container::flat_set<T, P, allocator<T>>;

}
}
