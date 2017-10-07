// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <memory>
#include <type_traits>
#include <algorithm>

#include <aligned_allocator.hpp>

#include <new>

namespace ste {

template <
	typename T, 
	std::size_t alignment = std::hardware_destructive_interference_size,
	typename AlignedAllocator = aligned_allocator<std::uint8_t, alignment>
>
class aligned_padded_ptr {
public:
	using value_type = T;
	using allocator_type = AlignedAllocator;

private:
	static constexpr bool pads_to_alignment = true;
	static constexpr std::size_t block_size = sizeof(value_type) + (pads_to_alignment ? alignment - (sizeof(value_type) % alignment) % alignment : 0u);

	template <typename... Ts>
	static void make(void *p,
					 Ts&&... ts) {
		auto ptr = static_cast<value_type*>(p);
		::new (ptr) value_type(std::forward<Ts>(ts)...);
	}

	static void deleter(value_type *ptr) {
		ptr->~value_type();
	}

private:
	T *ptr;
	AlignedAllocator allocator;

public:
	template <typename... Ts>
	aligned_padded_ptr(Ts&&... ts)
		: ptr(reinterpret_cast<T*>(allocator.allocate(block_size)))
	{
		make(ptr, std::forward<Ts>(ts)...);
	}
	// msvc 14.10.25017 (VC++ 2017 March release) fails with an internal error with conditional noexcept
//	~aligned_ptr() noexcept(std::is_nothrow_destructible_v<value_type>) {}
	~aligned_padded_ptr() noexcept {
		if (ptr) {
			deleter(ptr);
			allocator.deallocate(reinterpret_cast<std::uint8_t*>(ptr), block_size);
		}
	}

	// TODO: Respect allocator's propagate_on_container_move_assignment
	aligned_padded_ptr(aligned_padded_ptr &&o) noexcept : ptr(o.ptr), allocator(std::move(o.allocator)) {
		o.ptr = nullptr;
	}
	aligned_padded_ptr &operator=(aligned_padded_ptr &&o) noexcept {
		ptr = o.ptr;
		allocator = std::move(o.allocator);

		o.ptr = nullptr;

		return *this;
	}

	auto *get() { return ptr; }
	auto *get() const { return ptr; }
	auto *operator->() { return get(); }
	auto *operator->() const { return get(); }
	auto &operator*() { return *get(); }
	auto &operator*() const { return *get(); }
};

}
