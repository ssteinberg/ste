//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <is_anchored.hpp>

namespace ste {

template <typename T>
class alias {
private:
	T *ptr;

public:
	alias(T &object) noexcept : ptr(&object) {}

	alias(alias&&) noexcept = default;
	alias(const alias&) noexcept = default;
	alias &operator=(alias&&) noexcept = default;
	alias &operator=(const alias&) noexcept = default;

	alias(T *p) noexcept : ptr(p) {}
	alias &operator=(T *p) noexcept {
		ptr = p;
		return *this;
	}

	bool operator==(const alias<T> &rhs) const { return ptr == rhs.ptr; }
	bool operator!=(const alias<T> &rhs) const { return ptr != rhs.ptr; }

	auto& get() noexcept {
		static_assert(is_anchored_v<T>, "T must be anchored"); 
		return *ptr; 
	}
	const auto& get() const noexcept {
		static_assert(is_anchored_v<T>, "T must be anchored"); 
		return *ptr; 
	}

	T* operator->() noexcept { return &get(); }
	const T* operator->() const noexcept { return &get(); }
	T& operator*() noexcept { return get(); }
	const T& operator*() const noexcept { return get(); }

	operator T&() noexcept { return get(); }
	operator const T&() const noexcept { return get(); }
};

}
