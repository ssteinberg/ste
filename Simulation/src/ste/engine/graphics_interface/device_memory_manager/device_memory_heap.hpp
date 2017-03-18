//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_device_memory.hpp>
#include <device_memory_block.hpp>
#include <device_memory_exceptions.hpp>
#include <unique_device_ptr.hpp>

#include <list>
#include <mutex>

namespace StE {
namespace GL {

class device_memory_heap {
public:
	using size_type = std::uint64_t;
	using block_type = device_memory_block;
	using blocks_list_t = std::list<block_type>;
	using allocation_type = unique_device_ptr<device_memory_heap, blocks_list_t::iterator>;

	// Natively align allocations on vec4 boundaries
	static constexpr size_type base_alignment = 128;

private:
	friend class allocation_type;

private:
	vk_device_memory memory;
	blocks_list_t blocks;
	size_type total_used_size{ 0 };

	std::mutex m;

public:
	static auto align_round_down(size_type offset, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return (offset / alignment) * alignment;
	}
	static auto align(size_type size, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return ((size + alignment - 1) / alignment) * alignment;
	}

private:
	void deallocate(const allocation_type &ptr) {
		std::unique_lock<std::mutex> lock(m);

		total_used_size -= (*ptr).get_bytes();
		blocks.erase(ptr.get());
	}

	device_memory_heap(std::unique_lock<std::mutex> &&,
					   device_memory_heap *o) noexcept
		: memory(std::move(o->memory)),
		blocks(std::move(o->blocks)),
		total_used_size(o->total_used_size)
	{
		o->total_used_size = 0;
	}

public:
	device_memory_heap(vk_device_memory &&m) : memory(std::move(m)) {}

	// Move contrsuctor. Lock moved heaap's mutex before moving.
	device_memory_heap(device_memory_heap &&o) noexcept
		: device_memory_heap(std::unique_lock<std::mutex>(o.m), &o)
	{}

	device_memory_heap &operator=(device_memory_heap &&) = delete;

	/**
	*	@brief	Attempts to allocate memory. Returns an empty allocation on error.
	*			Thread safe.
	*
	*	@param size			Allocation size in bytes
	*	@param alignment	Allocation alignment
	*	
	*	@return	Returns the allocation object
	*/
	allocation_type allocate(size_type size, size_type alignment, bool private_allocation) {
		size = align(size, alignment);

		if (get_heap_size() < size) {
			// Heap too small
			return allocation_type();
		}

		std::unique_lock<std::mutex> lock(m);

		std::uint64_t start = 0;
		auto it = blocks.begin();
		for (;;) {
			bool last = it == blocks.end();

			auto end = memory.get_size();
			decltype(end) len;
			if (!last) {
				end = it->get_offset();
				len = it->get_bytes();
				++it;
			}

			start = align(start, alignment);
			if (end > start &&
				end - start >= size) {
				// Allocate
				device_memory_block block(start, size);

				auto block_it = blocks.insert(it, block);
				total_used_size += size;

				return allocation_type(&memory, 
									   this, 
									   std::move(block_it), 
									   private_allocation);
			}

			if (last)
				break;
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
};

}
}
