//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_handle.hpp>
#include <vk_device_memory.hpp>
#include <device_memory_block.hpp>
#include <device_memory_exceptions.hpp>
#include <unique_device_ptr.hpp>

#include <lib/vector.hpp>
#include <mutex>
#include <aligned_ptr.hpp>

namespace ste {
namespace gl {

class device_memory_heap {
public:
	using allocator_t = unique_device_ptr_allocator;

	using size_type = std::uint64_t;
	using block_type = device_memory_block<size_type>;
	using blocks_list_t = lib::vector<block_type>;
	using allocation_type = unique_device_ptr;

	// Natively align allocations on vec4 boundaries
	static constexpr size_type base_alignment = 128;

private:
	const allocator_t *owner;
	std::uint32_t memory_type;

	vk::vk_device_memory memory;

	blocks_list_t blocks;
	size_type total_used_size{ 0 };

public:
	static auto align_round_down(size_type offset, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return (offset / alignment) * alignment;
	}
	static auto align(size_type size, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return ((size + alignment - 1) / alignment) * alignment;
	}

public:
	device_memory_heap(const allocator_t *owner,
					   std::uint32_t memory_type,
					   vk::vk_device_memory &&m)
		: owner(owner), memory_type(memory_type), memory(std::move(m))
	{}

	device_memory_heap(device_memory_heap &&) = delete;
	device_memory_heap &operator=(device_memory_heap &&) = delete;

	/**
	*	@brief	Deallocates an allocation
	*/
	void deallocate(const allocation_type &ptr) {
		total_used_size -= ptr->get_bytes();
		auto it = std::lower_bound(blocks.begin(), blocks.end(), *ptr);

		assert(*it == *ptr);
		blocks.erase(it);
	}

	/**
	*	@brief	Attempts to allocate memory. Returns an empty allocation on error.
	*
	*	@param size			Allocation size in bytes
	*	@param alignment		Allocation alignment
	*	
	*	@return	Returns the allocation object
	*/
	allocation_type allocate(size_type size, size_type alignment, bool private_allocation) {
		size = align(size, alignment);

		if (get_heap_size() < size) {
			// Heap too small
			return allocation_type();
		}

		size_type start = 0;
		auto it = blocks.begin();
		for (;;) {
			bool last = it == blocks.end();

			auto end = memory.get_size();
			decltype(end) len;
			if (!last) {
				end = it->get_offset();
				len = it->get_bytes();
			}

			start = align(start, alignment);
			if (end > start &&
				end - start >= size) {
				// Allocate
				block_type block(start, size);

				blocks.insert(it, block);
				total_used_size += size;

				return allocation_type(&memory, 
									   owner, 
									   block,
									   private_allocation,
									   memory_type,
									   tag());
			}

			if (last)
				break;

			++it;
			start = end + len;
		}

		// Too fragmented to fit
		return allocation_type();
	}

	auto& get_device_memory() const { return memory; }

	/**
	*	@brief	Returns the amount of allocated memory, in bytes, from this heap.
	*/
	size_type get_allocated_bytes() const { return total_used_size; }
	/**
	*	@brief	Returns the heap size, in bytes.
	*/
	size_type get_heap_size() const { return memory.get_size(); }

	std::uint64_t tag() const { return vk::vk_handle(get_device_memory()); }
};

}
}
