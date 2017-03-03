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
	using allocation_type = unique_device_ptr<device_memory_heap, blocks_list_t::const_iterator>;

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
	static auto align_offset(size_type offset, size_type alignment) {
		alignment = std::max(alignment, base_alignment);
		return (offset / alignment) * alignment;
	}
	static auto align_size(size_type size, size_type alignment) {
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
	{}

public:
	device_memory_heap(vk_device_memory &&m) : memory(std::move(m)) {}

	device_memory_heap(device_memory_heap &&o) noexcept
		: device_memory_heap(std::unique_lock<std::mutex>(o.m), &o)
	{}

	/**
	*	@brief	Attempts to allocate memory. Returns an empty allocation on error.
	*			Thread safe.
	*
	*	@param size	Allocation size in bytes
	*/
	allocation_type allocate(size_type size, size_type alignment) {
		if (get_size() < size) {
			// Heap too small
			return allocation_type();
		}

		std::unique_lock<std::mutex> lock(m);

		auto start = 0;
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

			assert(end >= start);

			if (end - start >= size) {
				// Allocate
				device_memory_block block(start, size);

				auto block_it = blocks.insert(it, block);
				total_used_size += size;

				return allocation_type(&memory, this, std::move(block_it));
			}

			if (last)
				break;
			start = end + len;
		}

		// Too fragmented to fit
		return allocation_type();
	}

	auto& get_device_memory() const { return memory; }
	size_type get_total_allocated_size() const { return total_used_size; }
	size_type get_size() const { return memory.get_size(); }
};

}
}
