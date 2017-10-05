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

namespace ste {
namespace gl {

class device_memory_heap {
public:
	using allocator_t = unique_device_ptr_allocator;

	using size_type = std::uint64_t;
	using block_type = device_memory_block;
	using blocks_list_t = lib::vector<block_type>;
	using allocation_type = unique_device_ptr;

	// Natively align allocations on vec4 boundaries
	static constexpr size_type base_alignment = 128;

private:
	const allocator_t *owner;
	std::uint32_t memory_type;

	vk::vk_device_memory<> memory;

	blocks_list_t blocks;
	byte_t total_used_size{ 0 };

public:
	static auto align_round_down(byte_t offset, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return offset / alignment * alignment;
	}
	static auto align(byte_t size, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return ((size + byte_t(alignment) - 1_B) / alignment) * alignment;
	}

public:
	device_memory_heap(const allocator_t *owner,
					   std::uint32_t memory_type,
					   vk::vk_device_memory<> &&m)
		: owner(owner), memory_type(memory_type), memory(std::move(m))
	{}

	device_memory_heap(device_memory_heap &&) = delete;
	device_memory_heap &operator=(device_memory_heap &&) = delete;

	/**
	*	@brief	Deallocates an allocation
	*/
	void deallocate(const allocation_type &ptr) {
		total_used_size -= ptr->get_bytes();
		const auto it = std::lower_bound(blocks.begin(), blocks.end(), *ptr);

		if (*it == *ptr) {
			blocks.erase(it);
			return;
		}

		// Allocation not found in this heap!
		assert(false);
	}

	/**
	*	@brief	Attempts to allocate memory. Returns an empty allocation on error.
	*
	*	@param bytes			Allocation size in bytes
	*	@param alignment		Allocation alignment
	*	
	*	@return	Returns the allocation object
	*/
	allocation_type allocate(byte_t bytes, 
							 size_type alignment, 
							 bool private_allocation) {
		bytes = align(bytes, alignment);

		if (get_heap_size() < bytes) {
			// Heap too small
			return allocation_type();
		}

		byte_t start = 0_B;
		auto it = blocks.begin();
		for (;;) {
			const bool last = it == blocks.end();

			auto end = memory.get_size();
			decltype(end) len;
			if (!last) {
				end = it->get_offset();
				len = it->get_bytes();
			}

			start = align(start, alignment);
			if (end > start &&
				end - start >= bytes) {
				// Allocate
				const block_type block(start, bytes);

				blocks.insert(it, block);
				total_used_size += bytes;

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
	byte_t get_allocated_bytes() const { return total_used_size; }
	/**
	*	@brief	Returns the heap size, in bytes.
	*/
	byte_t get_heap_size() const { return memory.get_size(); }

	std::uint64_t tag() const { return vk::vk_handle(get_device_memory()); }
};

}
}
