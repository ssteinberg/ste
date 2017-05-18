//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

namespace ste {
namespace lib {

template <typename T, typename... Args>
void construct(T *ptr, Args&&... args) {
	::new (ptr) T(std::forward<Args>(args)...);
}

template <typename T>
void desctruct(T *ptr) {
	ptr->~T();
}

template <typename T, typename... Args>
auto allocate_construct(Args&&... args) {
	auto ptr = allocator<T>().allocate(1);
	::new (ptr) T(std::forward<Args>(args)...);

	return ptr;
}

template <typename T>
void desctruct_deallocate(T *ptr) {
	if (ptr) {
		ptr->~T();
		allocator<T>().deallocate(ptr, 1);
	}
}

}
}
