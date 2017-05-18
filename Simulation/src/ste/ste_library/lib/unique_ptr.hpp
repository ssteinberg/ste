//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <memory>
#include <allocator_delete.hpp>

namespace ste {
namespace lib {

template <typename T>
using unique_ptr = std::unique_ptr<T, allocator_delete<allocator<T>>>;

template <typename T, typename... Args>
auto allocate_unique(Args&&... args) {
	auto ptr = allocator<T>().allocate(1);
	::new (ptr) T(std::forward<Args>(args)...);

	return unique_ptr<T>(ptr);
}


}
}
