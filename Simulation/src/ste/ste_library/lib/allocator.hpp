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

template <typename T, std::size_t Align = alignof(std::max_align_t)>
class allocator : private allocator_base {
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	static constexpr auto alignment = Align;

	using pointer = T*;
	using const_pointer = const T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;

private:
	// rpmalloc allocates with 16byte base alignment
	static constexpr std::size_t _base_allocator_align = 16;
	static constexpr auto _use_aligned_alloc = alignment > 16;

public:
	template<class U>
	struct rebind {
		using other = allocator<U, Align>;
	};

	constexpr allocator() noexcept = default;
	template <typename U>
	allocator(const allocator<U, Align>&) noexcept {}

	auto allocate(size_type n) {
		return reinterpret_cast<T*>(
			_use_aligned_alloc ?
				rpaligned_alloc(alignment, sizeof(T) * n) :
				rpmalloc(sizeof(T) * n)
		);
	}
	void deallocate(T *p, size_type) {
		return rpfree(p);
	}

	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator==(const allocator<U1, A1>&, const allocator<U2, A2>&) noexcept;
};

template <
	typename U1, std::size_t A1,
	typename U2, std::size_t A2
>
bool inline operator==(const allocator<U1, A1>&, const allocator<U2, A2>&) noexcept {
	return A1 == A2;
}

}
}
