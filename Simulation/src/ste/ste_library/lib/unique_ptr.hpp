//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <lib/alloc.hpp>

#include <memory>
#include <lib/allocator_delete.hpp>

namespace ste {
namespace lib {

template <typename T>
using unique_ptr = std::unique_ptr<T, allocator_delete<std::remove_const_t<T>>>;

template <typename T, typename... Args>
auto allocate_unique(Args&&... args) {
	using non_const_t = std::remove_const_t<T>;
	using A = allocator<non_const_t>;

	auto ptr = alloc<A>::make(std::forward<Args>(args)...);
	return ::ste::lib::unique_ptr<T>(ptr);
}

}
}
