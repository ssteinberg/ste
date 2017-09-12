//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstddef>
#include <ltalloc.h>

#include <type_traits>
#include <cstring>

namespace ste {
namespace lib {

template <
	typename T,
	std::size_t Align = alignof(std::max_align_t)
>
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

	using reference = value_type&;
	using const_reference = const value_type&;

	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::false_type;

private:
	// Allocation base alignment
	static constexpr std::size_t _base_allocator_align = 4;
	static constexpr auto _use_aligned_alloc = alignment > _base_allocator_align;

private:
	auto bytes(size_type n) { return sizeof(T) * n; }

public:
	template <class U,
			  std::size_t A = Align>
	struct rebind {
		using other = allocator<U, A>;
	};

	constexpr allocator() noexcept = default;

	template <typename U>
	allocator(const allocator<U, Align> &) noexcept {}

	/*
	*	@brief	Allocates a block of memory
	*
	*	@param	n			Element count to allocate.
	*/
	T *allocate(size_type n) {
		if constexpr (_use_aligned_alloc)
			return allocate_aligned(n, alignment);

		auto p = ltalloc(bytes(n));
		if (p == nullptr)
			throw std::bad_alloc();

		return reinterpret_cast<T*>(p);
	}

	/*
	*	@brief	Deallocates memory
	*
	*	@param	p			Pointer to a block, previously allocated by this allocator, that will be deallocated
	*/
	void deallocate(pointer p, size_type) noexcept {
		static_assert(!std::is_const_v<T>, "const allocators are ill-formed.");

		ltfree(static_cast<void*>(p));
	}

	/*
	*	@brief	Deallocates memory
	*
	*	@param	p			Pointer to a block, previously allocated by this allocator, that will be deallocated
	*/
	void deallocate(pointer p) noexcept {
		deallocate(p, 1);
	}

	/*
	 *	@brief	Allocates a block of aligned memory
	 *			
	 *	@param	n			Element count to allocate.
	 *	@param	alignment	Alignment. Must be a power-of-two.
	 */
	T *allocate_aligned(size_type n, std::size_t alignment) {
		assert(alignment > 0 && ((alignment - 1) & alignment) == 0 && "alignment 0 or not a power-of-two");

		const auto mask = alignment - 1;
		const auto allocation_size = (bytes(n) + mask) & ~mask;

		auto p = ltalloc(allocation_size);
		if (p == nullptr)
			throw std::bad_alloc();
		assert(reinterpret_cast<std::uint64_t>(p) % alignment == 0);

		return reinterpret_cast<T*>(p);
	}

	/*
	*	@brief	Reallocates memory. 
	*			The content is preserved up to the lesser of the old and new sizes.
	*
	*	@param	p			Pointer to a block previously allocated by this allocator
	*	@param	n			New element count for the memory block.
	*/
	T *reallocate(pointer p, size_type n) {
		// Allocate new block amd copy
		auto t = n > 0 ? allocate(n) : nullptr;
		if (t && p)
			std::memcpy(t, p, std::min(allocation_useable_size(p), bytes(n)));
		deallocate(p);

		return reinterpret_cast<T*>(t);
	}

	/*
	*	@brief	Reallocates aligned memory.
	*			The content is preserved up to the lesser of the old and new sizes.
	*
	*	@param	p			Pointer to a block previously allocated by this allocator
	*	@param	n			New element count for the memory block.
	*	@param	alignment	Alignment. Must be a power-of-two.
	*/
	T *reallocate_aligned(pointer p, size_type n, std::size_t alignment) {
		auto t = n > 0 ? allocate_aligned(n, alignment) : nullptr;
		if (t && p)
			std::memcpy(t, p, std::min(allocation_useable_size(p), bytes(n)));
		deallocate(p);

		return reinterpret_cast<T*>(t);
	}

	/**
	 *	@brief	Returns the size of an allocated memory block, in bytes.
	 */
	static size_type allocation_useable_size(pointer p) noexcept {
		return ltalloc_usable_size(static_cast<void*>(p));
	}

	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator==(const allocator<U1, A1> &, const allocator<U2, A2> &) noexcept;
	template <
		typename U1, std::size_t A1,
		typename U2, std::size_t A2
	>
	friend bool operator!=(const allocator<U1, A1> &, const allocator<U2, A2> &) noexcept;
};

template <
	typename U1, std::size_t A1,
	typename U2, std::size_t A2
>
bool inline operator==(const allocator<U1, A1> &, const allocator<U2, A2> &) noexcept {
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
