//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <lib/unique_ptr.hpp>

#include <concurrency/concurrent_queue.hpp>

namespace ste {
namespace lib {

template <typename T>
using concurrent_queue = ::ste::concurrent_queue<T, allocator<T>, unique_ptr<T>>;

}
}
