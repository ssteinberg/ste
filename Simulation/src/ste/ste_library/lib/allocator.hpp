//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstddef>
#include <rpmalloc/rpmalloc.h>

#include <type_traits>

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

struct allocator_static_storage {
	static void init() {
		static _allocator_rpmalloc_init _rpmalloc_init;
		static thread_local _allocator_rpmalloc_thread_init _rpmalloc_thread_init;
	}
};

}

template <typename T, std::size_t Align = alignof(std::max_align_t)>
class allocator {
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	static constexpr auto alignment = Align;

	using pointer = value_type*;
	using const_pointer = const value_type*;
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
		_internal::allocator_static_storage::init();
		return reinterpret_cast<T*>(
			_use_aligned_alloc ?
				rpaligned_alloc(alignment, sizeof(T) * n) :
				rpmalloc(sizeof(T) * n)
		);
	}
	void deallocate(pointer p, size_type) {
		static_assert(!std::is_const_v<T>, "const allocators are ill-formed.");

		_internal::allocator_static_storage::init();
		return rpfree(static_cast<void*>(p));
	}
	void deallocate(pointer p) {
		deallocate(p, 1);
	}

	size_type allocation_useable_size(pointer p) {
		return rpmalloc_usable_size(static_cast<void*>(p));
	}

	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator==(const allocator<U1, A1>&, const allocator<U2, A2>&) noexcept;
	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator!=(const allocator<U1, A1>&, const allocator<U2, A2>&) noexcept;
};

template <
	typename U1, std::size_t A1,
	typename U2, std::size_t A2
>
bool inline operator==(const allocator<U1, A1>&, const allocator<U2, A2>&) noexcept {
	return A1 == A2;
}
template <
	typename U1, std::size_t A1,
	typename U2, std::size_t A2
>
bool inline operator!=(const allocator<U1, A1> &lhs, const allocator<U2, A2> &rhs) noexcept {
	return !(rhs == lhs);
}

}
}
