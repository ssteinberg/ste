//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_resource.hpp>

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
	using chunks_t = std::vector<std::unique_ptr<chunk_t>>;
	using allocation_t = chunk_t::allocation_type;
	using heap_index_t = std::uint32_t;

	struct heap_t {
		chunks_t chunks;
		chunks_t private_chunks;
		std::mutex m;
	};

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

	static void prune_chunks(chunks_t &chunks) {
		for (auto i = 0; i < chunks.size(); ++i) {
			auto &h = chunks[i];

			// Destroy unallocated chunks
			if (h->get_total_allocated_size() == 0) {
				chunks.erase(chunks.begin() + i);
				--i;
			}
		}
	}
	static void prune_heap(heap_t &heap) {
		prune_chunks(heap.chunks);
		prune_chunks(heap.private_chunks);
	}

	static auto allocate_from_heap(heap_t &heap, std::uint64_t size, std::uint64_t alignment) {
		allocation_t allocation;
		for (auto &h : heap.chunks) {
			allocation = h->allocate(size, alignment);
			if (allocation) {
				// On successful allocation, prune the heap
				prune_heap(heap);
				break;
			}
		}

		return allocation;
	}

	auto create_chunk_for_heap_index(const heap_index_t &heap_idx,
									 std::uint64_t minimal_size,
									 bool private_memory) const {
		// Calculate chunk size
		auto chunk_size = private_memory ?
			minimal_size :
			std::max(minimal_size, minimal_allocation_size_bytes);
		// Align it
		chunk_size = device_memory_heap::align_size(chunk_size, 0);

		// Create the device memory object
		auto memory = vk_device_memory(device, chunk_size, heap_idx);
		// And use it to create the chunk
		return std::make_unique<chunk_t>(std::move(memory));
	}

	auto allocate_device_memory(const VkMemoryRequirements &memory_requirements,
								const VkMemoryPropertyFlags &required_flags,
								const VkMemoryPropertyFlags &preferred_flags,
								bool private_memory = false) const {
		auto heap_idx = find_heap_index_for(device, memory_requirements, required_flags, preferred_flags | required_flags);
		auto& heap = heaps[heap_idx];

		auto size = memory_requirements.size;
		auto alignment = memory_requirements.alignment;

		std::unique_lock<std::mutex> lock(heap.m);

		if (heap.chunks.size()) {
			// Try to allocate memory on one of the existing chunks
			auto allocation = allocate_from_heap(heap, size, alignment);
			if (allocation)
				return allocation;
		}

		// Need to create a new chunk
		auto chunk = create_chunk_for_heap_index(heap_idx, memory_requirements.size, private_memory);
		auto allocation = chunk->allocate(size, alignment);
		if (private_memory)
			heap.private_chunks.push_back(std::move(chunk));
		else
			heap.chunks.push_back(std::move(chunk));

		// On successful allocation, prune the heap
		prune_heap(heap);

		assert(allocation);
		return allocation;
	}

public:
	ste_gl_device_memory_allocator(const vk_logical_device &device) : device(device) {}

	/**
	*	@brief	Attempts to allocate memory and bind that memory to the resource.
	*			Thread safe.
	*
	*	@param resource			The resource
	*	@param required_flags	Required memory flags. 
	*							If some of the bits can't be satisfied, allocation will throw vk_memory_no_supported_heap_exception.
	*	@param preferred_flags	Nice to have flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this resource, i.e. this chunk will not
	*							be shared with other resources. Useful for resources whose underlying memory might need to be host mapped.
	*/
	void allocate_device_memory_for_resource(vk_resource &resource,
											 const VkMemoryPropertyFlags &required_flags,
											 const VkMemoryPropertyFlags &preferred_flags,
											 bool private_memory = false) const {
		auto memory_requirements = resource.get_memory_requirements();
		auto allocation = allocate_device_memory(memory_requirements,
												 required_flags,
												 preferred_flags,
												 private_memory);

		if (!allocation) {
			throw device_memory_allocation_failed();
		}

		resource.bind_memory(allocation.get_memory(), private_memory, (*allocation).get_offset());
	}

	/**
	*	@brief	Attempts to allocate device local memory and bind that memory to the resource.
	*			Thread safe.
	*
	*	@param resource			The resource
	*	@param required_flags	Required memory flags.
	*							If some of the bits can't be satisfied, allocation will throw vk_memory_no_supported_heap_exception.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this resource, i.e. this chunk will not
	*							be shared with other resources. Useful for resources whose underlying memory might need to be host mapped.
	*/
	void allocate_device_physical_memory_for_resource(vk_resource &resource,
													  const VkMemoryPropertyFlags &required_flags = 0,
													  bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		allocate_device_memory_for_resource(resource, required_flags, preferred_flags, private_memory);
	}

	/**
	*	@brief	Attempts to allocate host visible memory and bind that memory to the resource.
	*			Thread safe.
	*
	*	@param resource			The resource
	*	@param required_flags	Required memory flags.
	*							If some of the bits can't be satisfied, allocation will throw vk_memory_no_supported_heap_exception.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this resource, i.e. this chunk will not
	*							be shared with other resources. Useful for resources whose underlying memory might need to be host mapped.
	*/
	void allocate_host_visible_memory_for_resource(vk_resource &resource,
												   const VkMemoryPropertyFlags &required_flags = 0,
												   bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		allocate_device_memory_for_resource(resource, required_flags, preferred_flags, private_memory);
	}
};

}
}
