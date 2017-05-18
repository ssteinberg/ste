//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {

template <typename T>
class aligned_allocator {
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
		using other = aligned_allocator<U>;
	};

	aligned_allocator() = default;

	auto aligned_allocate(size_type n, size_type alignment) {
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
	void aligned_deallocate(T *ptr, size_type) {
#ifdef _MSC_VER
		_aligned_free(ptr);
#elif defined _linux
		free(ptr);
#endif
	}

	bool operator==(const aligned_allocator<T> &) const { return true; }
	bool operator!=(const aligned_allocator<T> &) const { return false; }
};

}
