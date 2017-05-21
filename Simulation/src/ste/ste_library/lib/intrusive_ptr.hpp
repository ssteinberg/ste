//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <lib/alloc.hpp>

#include <boost/intrusive_ptr.hpp>

namespace ste {
namespace lib {

template <typename T>
using intrusive_ptr = boost::intrusive_ptr<T>;

template <typename T, typename... Args>
auto allocate_intrusive(Args&&... args) {
	static_assert(!std::is_array_v<T>, "Arrays not supported");
	
	auto ptr = default_alloc<T>::make(std::forward<Args>(args)...);
	return intrusive_ptr<T>(ptr);
}

}
}
