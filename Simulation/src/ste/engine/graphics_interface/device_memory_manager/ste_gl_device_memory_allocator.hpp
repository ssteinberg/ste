//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_memory_exception.hpp>
#include <vk_logical_device.hpp>
#include <vk_device_memory.hpp>
#include <device_memory_heap.hpp>

#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>

namespace StE {
namespace GL {

class ste_gl_device_memory_allocator {
private:
	static constexpr std::uint64_t minimal_allocation_size_bytes = 128 * 1024 * 1024;

	using chunk_t = device_memory_heap;
	using allocation_t = chunk_t::allocation_type;
	struct heap_t {
		std::vector<std::unique_ptr<chunk_t>> chunks;
		std::mutex m;
	};
	using heap_index_t = std::uint32_t;

private:
	const vk_logical_device &device;
	mutable std::unordered_map<heap_index_t, heap_t> heaps;

private:
	static heap_index_t find_heap_index_for(const vk_logical_device &device,
											const VkMemoryPropertyFlags &required_flags) {
		const vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();

		// Try to find a heap that matches exactly
		for (int i = 0; i < physical_device.memory_properties.memoryTypeCount; ++i) {
			auto &heap = physical_device.memory_properties.memoryTypes[i];
			if (heap.propertyFlags == required_flags)
				return heap.heapIndex;
		}

		// Otherwise try to find a heap that satisfies all flags
		for (int i = 0; i < physical_device.memory_properties.memoryTypeCount; ++i) {
			auto &heap = physical_device.memory_properties.memoryTypes[i];
			if ((heap.propertyFlags & required_flags) == required_flags)
				return heap.heapIndex;
		}

		// No heap with requested flags found
		throw vk_memory_no_supported_heap_exception();
	}

	static heap_index_t find_heap_index_for(const vk_logical_device &device,
											const VkMemoryRequirements &memory_requirements,
											const VkMemoryPropertyFlags &required_flags,
											const VkMemoryPropertyFlags &preferred_flags) {
		const vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();
		int fallback_heap_index = -1;

		// Try to find a heap matching the memory requirments and preffered flags.
		// If none found fallback to a heap matching the requirements and the required flags.
		for (int type = 0; type < 32; ++type) {
			if (!(memory_requirements.memoryTypeBits & (1 << type)))
				continue;

			auto &heap = physical_device.memory_properties.memoryTypes[type];
			if ((heap.propertyFlags & preferred_flags) == preferred_flags)
				return heap.heapIndex;
			if (fallback_heap_index == -1 &&
				(heap.propertyFlags & required_flags) == required_flags)
				fallback_heap_index = heap.heapIndex;
		}

		if (fallback_heap_index != -1)
			return fallback_heap_index;

		// No heap with requested flags found
		throw vk_memory_no_supported_heap_exception();
	}

	static void prune_heap(heap_t &heap) {
		for (auto i = 0; i < heap.chunks.size(); ++i) {
			auto &h = heap.chunks[i];
			if (h->get_total_allocated_size() == 0) {
				heap.chunks.erase(heap.chunks.begin() + i);
				--i;
			}
		}
	}

	static auto allocate_from_heap(heap_t &heap, std::uint64_t size) {
		allocation_t allocation;
		{
			for (auto &h : heap.chunks) {
				allocation = h->allocate(size);
				if (allocation) {
					// On successful allocation, prune the heap
					prune_heap(heap);
					break;
				}
			}
		}

		return allocation;
	}

public:
	ste_gl_device_memory_allocator(const vk_logical_device &device) : device(device) {}

	auto allocate_device_memory(const VkMemoryRequirements &memory_requirements,
								const VkMemoryPropertyFlags &required_flags,
								const VkMemoryPropertyFlags &preferred_flags,
								std::uint64_t size = 0,
								bool tight_fit = false) const {
		assert((required_flags & preferred_flags) == required_flags && "preferred_flags must contain required_flags");

		auto heap_idx = find_heap_index_for(device, memory_requirements, required_flags, preferred_flags);
		auto& heap = heaps[heap_idx];

		std::unique_lock<std::mutex> lock(heap.m);

		if (heap.chunks.size()) {
			// Try to allocate memory on one of the existing chunks
			auto allocation = allocate_from_heap(heap, size);
			if (allocation)
				return allocation;
		}

		// Need to create a new chunk
		auto chunk_size = tight_fit ?
			memory_requirements.size :
			std::max(std::max(size, memory_requirements.size), minimal_allocation_size_bytes);
		auto memory = vk_device_memory(device, chunk_size, heap_idx);
		auto chunk = std::make_unique<chunk_t>(std::move(memory));

		auto allocation = chunk->allocate(size);
		heap.chunks.push_back(std::move(chunk));

		// On successful allocation, prune the heap
		prune_heap(heap);

		assert(allocation);
		return allocation;
	}
	auto allocate_device_physical_memory(const VkMemoryRequirements &memory_requirements,
										 const VkMemoryPropertyFlags &required_flags = 0,
										 std::uint64_t size = 0) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		return allocate_device_memory(memory_requirements, required_flags, preferred_flags, size);
	}
	auto allocate_host_visible_memory(const VkMemoryRequirements &memory_requirements,
									  const VkMemoryPropertyFlags &required_flags = 0,
									  std::uint64_t size = 0) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		return allocate_device_memory(memory_requirements, required_flags, preferred_flags, size);
	}
};

}
}
