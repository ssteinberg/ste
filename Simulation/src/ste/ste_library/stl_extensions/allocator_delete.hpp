//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {

template <class Allocator>
struct allocator_delete { 
	using T = typename Allocator::value_type;

	constexpr allocator_delete() noexcept = default;

	template<
		typename A2,
		typename = typename std::enable_if<std::is_convertible_v<typename A2::pointer, typename Allocator::pointer> && std::is_same_v<typename Allocator::template rebind<A2::value_type>::other, A2>>::type
	>
	allocator_delete(const allocator_delete<A2>&) noexcept {}

	void operator()(typename Allocator::pointer ptr) const noexcept {
		static_assert(sizeof(Allocator::value_type) > 0, "Can not delete an incomplete type");
		if (ptr) {
			ptr->~T();
			Allocator().deallocate(ptr, 1);
		}
	}
};

}
