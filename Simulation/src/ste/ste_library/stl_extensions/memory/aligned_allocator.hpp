//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#ifdef _linux
#include <stdlib.h>
#endif

namespace ste {

template <typename T, std::size_t Align = alignof(T)>
class aligned_allocator {
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	static constexpr auto alignment = Align;

	using pointer = T*;
	using const_pointer = const T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;

	template<class U>
	struct rebind {
		using other = aligned_allocator<U>;
	};

	aligned_allocator() = default;
	template <typename U>
	aligned_allocator(const aligned_allocator<U, Align>&) noexcept {}

	auto allocate(size_type n) {
#ifdef _MSC_VER
		return reinterpret_cast<T*>(_aligned_malloc(sizeof(T) * n, alignment));
#elif defined _linux
		void *ptr;
		posix_memalign(&ptr, alignment, sizeof(T) * n);
		return reinterpret_cast<T*>(ptr);
#else
#error Unsupported OS
#endif
	}
	void deallocate(T *ptr, size_type) {
#ifdef _MSC_VER
		_aligned_free(ptr);
#elif defined _linux
		free(ptr);
#endif
	}

	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator==(const aligned_allocator<U1, A1>&, const aligned_allocator<U2, A2>&) noexcept;
};

template <
	typename U1, std::size_t A1,
	typename U2, std::size_t A2
>
bool inline operator==(const aligned_allocator<U1, A1>&, const aligned_allocator<U2, A2>&) noexcept {
	return A1 == A2;
}

}
