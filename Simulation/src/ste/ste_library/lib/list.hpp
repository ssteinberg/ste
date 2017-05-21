//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <list>

namespace ste {
namespace lib {

template <typename T>
using list = std::list<T, allocator<T>>;

}
}
