//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <memory>

namespace ste {
namespace lib {

template <typename T>
using shared_ptr = std::shared_ptr<T>;

template <typename T, typename... Args>
shared_ptr<T> allocate_shared(Args&&... args) {
	return std::allocate_shared<T>(allocator<T>(), std::forward<Args>(args)...);
}

}
}
