//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rpmalloc/rpmalloc.h>

namespace ste {
namespace lib {

namespace _internal {

struct _allocator_rpmalloc_init {
	_allocator_rpmalloc_init() {
		rpmalloc_initialize();
	}
	~_allocator_rpmalloc_init() {
		rpmalloc_finalize();
	}
};

struct _allocator_rpmalloc_thread_init {
	_allocator_rpmalloc_thread_init() {
		rpmalloc_thread_initialize();
	}
	~_allocator_rpmalloc_thread_init() {
		rpmalloc_thread_finalize();
	}
};

}

class allocator_base {
	static _internal::_allocator_rpmalloc_init _rpmalloc_init;
	static thread_local _internal::_allocator_rpmalloc_thread_init _rpmalloc_thread_init;
};

template <typename T>
class allocator : private allocator_base {
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using pointer = T*;
	using const_pointer = const T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;

	template<class U>
	struct rebind {
		using other = allocator<U>;
	};

	allocator() = default;

	auto allocate(size_type n) {
		return reinterpret_cast<T*>(rpmalloc(sizeof(T) * n));
	}
	void deallocate(T *p, size_type) {
		return rpfree(p);
	}

	auto aligned_allocate(size_type n, size_type alignment) {
		return reinterpret_cast<T*>(rpaligned_alloc(alignment, sizeof(T) * n));
	}
	void aligned_deallocate(T *ptr, size_type n) {
		deallocate(ptr, n);
	}

	bool operator==(const allocator<T> &) const { return true; }
	bool operator!=(const allocator<T> &) const { return false; }
};

}
}
