//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <deque>

namespace ste {
namespace lib {

template <typename T>
using deque = std::deque<T, allocator<T>>;

}
}
