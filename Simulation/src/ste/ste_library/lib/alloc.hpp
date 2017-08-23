//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

namespace ste {
namespace lib {

namespace _detail {

template <typename Allocator, bool _is_array>
struct alloc_impl {};

template <typename Allocator>
struct alloc_impl<Allocator, false> {
	using T = typename Allocator::value_type;
	using U = std::remove_const_t<T>;
	using allocator = typename Allocator::template rebind<U>::other;

	using pointer = typename allocator::pointer;
	using const_pointer = typename allocator::const_pointer;

	template <typename Checker = U, typename... Args>
	static std::enable_if_t<!std::is_abstract_v<Checker>, pointer> ctor(pointer ptr,
																		Args&&... args) {
		::new (ptr) U(std::forward<Args>(args)...);
		return ptr;
	}

	static void dtor(const_pointer ptr) {
		static_assert(sizeof(U) > 0, "Can not delete an incomplete type");

		auto p = const_cast<pointer>(ptr);
		p->~U();
	}

	template <typename Checker = U, typename... Args>
	static std::enable_if_t<!std::is_abstract_v<Checker>, pointer> make(Args&&... args) {
		pointer ptr = allocator().allocate(1);
		return ctor(ptr, std::forward<Args>(args)...);
	}

	static void destroy(const_pointer ptr) {
		auto p = const_cast<pointer>(ptr);
		dtor(p);
		allocator().deallocate(p, 1);
	}
};

template <typename Allocator>
struct alloc_impl<Allocator, true> {
	using T = typename Allocator::value_type;

	using byte = std::uint8_t;
	using allocator = typename Allocator::template rebind<byte>::other;

	using D = std::decay_t<T>;
	using U = std::remove_const_t<D>;
	using value_type = std::remove_pointer_t<D>;

	using pointer = D;
	using const_pointer = const value_type*;
	using size_type = typename allocator::size_type;
	using length_type = size_type;

	static constexpr auto alignment = allocator::alignment;

	/**
	 *	@brief	Returns the expected allocation size in bytes to be used for an n-element array.
	 */
	static constexpr auto allocation_size_bytes(size_type n) {
		return array_length_size() + sizeof(D) * n;
	}

	template <typename Checker = value_type, typename... Args>
	static std::enable_if_t<!std::is_abstract_v<Checker>, pointer> ctor(pointer ptr, 
																		size_type n,
																		Args&&... args) {
		*reinterpret_cast<length_type*>(ptr) = static_cast<length_type>(n);

		reinterpret_cast<byte*&>(ptr) += array_length_size();
		for (size_type idx = 0; idx < n; ++idx)
			::new (&ptr[idx]) value_type(std::forward<Args>(args)...);

		return ptr;
	}

	static void dtor(const_pointer ptr) {
		static_assert(sizeof(D) > 0, "Can not delete an incomplete type");

		auto n = *reinterpret_cast<const length_type*>(reinterpret_cast<const byte*>(ptr) - array_length_size());

		auto p = const_cast<pointer>(ptr);
		for (auto i = n; i-- > 0;)
			ptr[i].~value_type();
	}

	template <typename Checker = value_type, typename... Args>
	static std::enable_if_t<!std::is_abstract_v<Checker>, pointer> make(size_type n, 
																		Args&&... args) {
		auto ptr = reinterpret_cast<pointer>(allocator().allocate(allocation_size_bytes(n)));
		return ctor(ptr, n, std::forward<Args>(args)...);
	}

	static void destroy(const_pointer ptr) {
		auto p = const_cast<pointer>(ptr);
		dtor(p);

		auto start = reinterpret_cast<byte*>(p) - array_length_size();
		allocator().deallocate(start);
	}

private:
	static constexpr auto array_length_size() {
		// Calculate the amount to allocate at the array start. Preserves alignment.
		return (sizeof(length_type) + alignment - 1) / alignment * alignment;
	}
};

}

template <typename Allocator>
using alloc = _detail::alloc_impl<Allocator, std::is_array_v<typename Allocator::value_type> && std::extent_v<typename Allocator::value_type> == 0>;

template <typename T>
using default_alloc = alloc<allocator<T>>;

}
}
