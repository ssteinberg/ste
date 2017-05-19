//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

namespace ste {
namespace lib {

template <typename T>
struct allocator_delete {
	using Allocator = allocator<T>;

	using pointer = typename Allocator::pointer;
	using const_pointer = typename Allocator::const_pointer;

	constexpr allocator_delete() noexcept = default;

	template<typename U>
	allocator_delete(const allocator_delete<U[]>&) = delete;
	template<
		typename U,
		typename = typename std::enable_if<!std::is_array_v<U> && std::is_convertible_v<typename allocator_delete<U>::pointer, pointer>>::type
	>
	allocator_delete(const allocator_delete<U>&) noexcept {}

	void operator()(const_pointer ptr) const noexcept {
		alloc<Allocator>::destroy(ptr);
	}
};

template <typename T>
struct allocator_delete<T[]> {
	using Allocator = allocator<T[]>;

	using pointer = T*;
	using const_pointer = const T*;

	constexpr allocator_delete() noexcept = default;

	template<
		typename U,
		typename = typename std::enable_if<std::is_convertible_v<typename allocator_delete<U>::pointer, pointer>>::type
	>
	allocator_delete(const allocator_delete<U[]>&) noexcept {}

	void operator()(const_pointer ptr) const noexcept {
		alloc<Allocator>::destroy(ptr);
	}
};

}
}
