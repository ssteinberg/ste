//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstddef>

#include <type_traits>

namespace ste {

/*
 *	@brief	Used by small_allocator
 *			In spirit of https://howardhinnant.github.io/stack_alloc.html (by Howard Hinnant)
 */
template <
	std::size_t bytes, 
	std::size_t alignment = alignof(std::max_align_t)
>
class small_arena {
private:
	static_assert(bytes % alignment == 0, "Pool size must be a multiple of aligment");

	using arena_type = std::uint8_t[bytes];

	alignas(alignment)arena_type pool;
	std::uint8_t *offset;
	std::uint8_t *prev_offset;

private:
	static auto align(std::size_t size) {
		return (size + alignment - 1) / alignment * alignment;
	}

public:
	small_arena() noexcept : offset(pool), prev_offset(offset) {}

	small_arena(small_arena&&) = delete;
	small_arena(const small_arena&) = delete;
	small_arena &operator=(small_arena&&) = delete;
	small_arena &operator=(const small_arena&) = default;

	bool can_allocate(std::size_t size) const noexcept {
		return static_cast<std::ptrdiff_t>(bytes) - (offset - pool) >= static_cast<std::ptrdiff_t>(align(size));
	}
	bool belongs_to_pool(const void* ptr) const {
		return ptr >= pool && ptr < offset;
	}
	void reset() noexcept { offset = pool; }

	void* allocate(std::size_t size) noexcept {
		size = align(size);

		prev_offset = offset;
		offset += size;
		assert(offset <= pool + bytes);

		return prev_offset;
	}
	void deallocate(void* ptr, std::size_t size) noexcept {
		size = align(size);

		// We keep 2 offset pointers, this allows us to keep track of the last 2 non-freed allocations, in sync with patterns like std::vector's resize which allocates a new 
		// storage and then deallocates the previous one.
		if (offset == reinterpret_cast<std::uint8_t*>(ptr) + size) {
			offset -= size;
			if (prev_offset != offset)
				offset = prev_offset;
		}
		else if (prev_offset == reinterpret_cast<std::uint8_t*>(ptr) + size)
			prev_offset -= size;
	}
};

template <
	typename T,
	std::size_t stack_size,
	typename DynamicAllocator = std::allocator<T>
>
class small_allocator {
	template <typename, std::size_t, typename>
	friend class small_allocator;

private:
	template <class U>
	using rebind_dynamic = typename DynamicAllocator::template rebind<U>::other;

public:
	using allocator_type = DynamicAllocator;
	using arena_type = small_arena<stack_size>;

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using pointer = value_type*;
	using const_pointer = const value_type*;
	using void_pointer = void*;
	using const_void_pointer = const void*;

	using reference = value_type&;
	using const_reference = const value_type&;

	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::false_type;

	static_assert(std::is_same_v<value_type, typename DynamicAllocator::value_type>, "DynamicAllocator not allocating type T");

private:
	arena_type& stack_pool;

	allocator_type dynamic_allocator;

public:
	template <class U>
	struct rebind {
		using other = small_allocator<U, stack_size, rebind_dynamic<U>>;
	};

	small_allocator(arena_type &pool) noexcept : stack_pool(pool) {}
	template <typename U>
	small_allocator(const small_allocator<U, stack_size, rebind_dynamic<U>> &o) noexcept : stack_pool(o.stack_pool), dynamic_allocator(o.dynamic_allocator) {}

	pointer allocate(size_type n) {
		auto size = sizeof(T) * n;
		if (stack_pool.can_allocate(size))
			return reinterpret_cast<pointer>(stack_pool.allocate(size));

		return dynamic_allocator.allocate(n);
	}
	void deallocate(pointer p, size_type n) {
		static_assert(!std::is_const_v<T>, "const allocators are ill-formed.");

		if (stack_pool.belongs_to_pool(p))
			stack_pool.deallocate(p, n);
		else
			dynamic_allocator.deallocate(p, n);
	}

	template <typename U2, std::size_t A2, typename D2>
	bool operator==(const small_allocator<U2, A2, D2> &rhs) const noexcept {
		return &stack_pool == &rhs.stack_pool && dynamic_allocator == rhs.dynamic_allocator;
	}
	template <typename U2, std::size_t A2, typename D2>
	bool operator!=(const small_allocator<U2, A2, D2> &rhs) const noexcept {
		return !(*this == rhs);
	}
};

}
