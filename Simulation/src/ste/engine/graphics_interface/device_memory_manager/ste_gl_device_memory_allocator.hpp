//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_memory_exception.hpp>
#include <vk_logical_device.hpp>
#include <vk_device_memory.hpp>
#include <device_memory_heap.hpp>

#include <mutex>
#include <memory>
#include <forward_list>
#include <array>
#include <unordered_set>

namespace StE {
namespace GL {

class ste_gl_device_memory_allocator {
private:
	using chunk_t = device_memory_heap;
	using chunks_t = std::forward_list<std::unique_ptr<chunk_t>>;
	using memory_type_t = std::uint32_t;

	struct heap_t {
		chunks_t chunks;
		chunks_t private_chunks;
		std::mutex m;
	};

	static constexpr memory_type_t memory_types = 32;

	using heap_ptr_t = std::unique_ptr<heap_t>;
	using heaps_t = std::array<heap_ptr_t, memory_types>;

public:
	// Allocate 256MB chunks by default
	static constexpr std::uint64_t default_minimal_allocation_size_bytes = 256 * 1024 * 1024;

	using allocation_t = chunk_t::allocation_type;

private:
	const vk_logical_device &device;
	const heaps_t heaps;
	std::uint64_t minimal_allocation_size_bytes;

private:
	static auto create_empty_heaps_map() {
		heaps_t h;
		for (memory_type_t type = 0; type < memory_types; ++type)
			h[type] = std::make_unique<heap_t>();
		return h;
	}

	static bool memory_type_matches_requirements(const memory_type_t &type,
												 const VkMemoryRequirements &memory_requirements) {
		return !!(memory_requirements.memoryTypeBits & (1 << type));
	}

	static memory_type_t find_memory_type_for(const vk_logical_device &device,
											  const VkMemoryRequirements &memory_requirements,
											  const VkMemoryPropertyFlags &required_flags,
											  const VkMemoryPropertyFlags &preferred_flags) {
		const vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();
		int fallback_memory_type = -1;

		// Try to find a heap matching the memory requirments and preferred flags.
		// If none found fallback to a heap matching the requirements and the required flags.
		for (memory_type_t type = 0; type < memory_types; ++type) {
			if (!memory_type_matches_requirements(type, memory_requirements))
				continue;

			auto &heap = physical_device.memory_properties.memoryTypes[type];
			if ((heap.propertyFlags & preferred_flags) == preferred_flags)
				return type;
			if (fallback_memory_type == -1 &&
				(heap.propertyFlags & required_flags) == required_flags)
				fallback_memory_type = type;
		}

		if (fallback_memory_type != -1)
			return fallback_memory_type;

		// No heap with requested flags found
		throw device_memory_no_supported_heap_exception();
	}

	static void prune_chunks(chunks_t &chunks) {
		auto before_it = chunks.before_begin();
		auto it = chunks.begin();
		while (it != chunks.end()) {
			auto &h = *it;

			// Destroy unallocated chunks
			if (h->get_allocated_bytes() == 0) {
				chunks.erase_after(before_it);
				it = before_it;
				++it;
			}
			else {
				before_it = it;
				++it;
			}
		}
	}
	static void prune_heap(heap_t &heap) {
		prune_chunks(heap.chunks);
		prune_chunks(heap.private_chunks);
	}

	static auto allocate_from_heap(heap_t &heap, 
								   std::uint64_t size, 
								   std::uint64_t alignment,
								   bool private_memory) {
		allocation_t allocation;
		for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it) {
			allocation = (*it)->allocate(size, alignment, private_memory);
			if (allocation) {
				// On successful allocation, prune the heap
				prune_heap(heap);
				break;
			}
		}

		return allocation;
	}

	auto commit_device_memory_heap_for_memory_type(const memory_type_t &memory_type,
												   std::uint64_t minimal_size,
												   std::uint64_t alignment,
												   bool private_memory) const {
		// Calculate chunk size
		auto chunk_size = private_memory ?
			minimal_size :
			std::max(minimal_size, minimal_allocation_size_bytes);
		// Align it
		chunk_size = device_memory_heap::align(chunk_size, alignment);

		// Create the device memory object
		return vk_device_memory(device, chunk_size, memory_type);
	}

	auto allocate(std::uint64_t size,
				  const VkMemoryRequirements &memory_requirements,
				  const VkMemoryPropertyFlags &required_flags,
				  const VkMemoryPropertyFlags &preferred_flags,
				  bool private_memory = false) const {
		auto memory_type = find_memory_type_for(device, memory_requirements, required_flags, preferred_flags | required_flags);
		auto alignment = memory_requirements.alignment;
		allocation_t allocation;

		assert(memory_type < heaps.size());
		auto &heap = *heaps[memory_type];

		{
			std::unique_lock<std::mutex> lock(heap.m);

			if (!heap.chunks.empty()) {
				// Try to allocate memory on one of the existing chunks
				allocation = allocate_from_heap(heap, 
												size, 
												alignment, 
												private_memory);
			}

			// If failed allocating on existing chunks, or no existing chunks available,
			// create a new chunk.
			if (!allocation) {
				chunks_t &chunks = private_memory ?
					heap.private_chunks :
					heap.chunks;

				// Commit device memory
				auto memory_object = commit_device_memory_heap_for_memory_type(memory_type, 
																			   size, 
																			   alignment, 
																			   private_memory);
				// And use it to create a new chunk
				auto chunk = std::make_unique<chunk_t>(std::move(memory_object));

				// Allocate from that chunk
				allocation = chunk->allocate(size, alignment, private_memory);

				chunks.push_front(std::move(chunk));

				// On commiting new chunks, prune the heap
				prune_heap(heap);
			}
		}

		assert(allocation);
		return allocation;
	}

public:
	ste_gl_device_memory_allocator(const vk_logical_device &device,
								   std::uint64_t minimal_allocation_size_bytes = default_minimal_allocation_size_bytes)
		: device(device), 
		heaps(create_empty_heaps_map()), 
		minimal_allocation_size_bytes(minimal_allocation_size_bytes)
	{}

	/**
	*	@brief	Attempts to allocate memory.
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param size				Size in bytes
	*	@param memory_requirements	Allocation memory requirements
	*	@param required_flags	Required memory flags.
	*	@param preferred_flags	Nice to have flags.
	*/
	auto allocate_device_memory(std::uint64_t size,
								const VkMemoryRequirements &memory_requirements,
								const VkMemoryPropertyFlags &required_flags,
								const VkMemoryPropertyFlags &preferred_flags,
								bool private_memory = false) const {
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags,
						false);
	}

	/**
	*	@brief	Attempts to allocate device local memory.
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param size				Size in bytes
	*	@param memory_requirements	Allocation memory requirements
	*	@param required_flags	Required memory flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this allocation, i.e. this chunk will not
	*							be shared with other alocation.
	*/
	auto allocate_device_physical_memory(std::uint64_t size,
										 const VkMemoryRequirements &memory_requirements,
										 const VkMemoryPropertyFlags &required_flags = 0,
										 bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags,
						private_memory);
	}

	/**
	*	@brief	Attempts to allocate host visible memory.
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param size				Size in bytes
	*	@param memory_requirements	Allocation memory requirements
	*	@param required_flags	Required memory flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this allocation, i.e. this chunk will not
	*							be shared with other alocation.
	*/
	auto allocate_host_visible_memory(std::uint64_t size,
									  const VkMemoryRequirements &memory_requirements,
									  const VkMemoryPropertyFlags &required_flags = 0,
									  bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags,
						private_memory);
	}

	/**
	*	@brief	Attempts to allocate memory based on vk resource memory requirements. 
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param memory_requirements	Memory requirements
	*	@param required_flags	Required memory flags.
	*	@param preferred_flags	Nice to have flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this allocation, i.e. this chunk will not
	*							be shared with other alocation.
	*/
	auto allocate_device_memory_for_resource(const VkMemoryRequirements &memory_requirements,
												 const VkMemoryPropertyFlags &required_flags,
												 const VkMemoryPropertyFlags &preferred_flags,
												 bool private_memory = false) const {
		auto allocation = allocate(memory_requirements.size,
								   memory_requirements,
								   required_flags,
								   preferred_flags,
								   private_memory);

		if (!allocation) {
			throw device_memory_allocation_failed();
		}

		return allocation;
	}

	/**
	*	@brief	Attempts to allocate device local memory and bind that memory to the resource.
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param memory_requirements	Memory requirements
	*	@param required_flags	Required memory flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this allocation, i.e. this chunk will not
	*							be shared with other alocation.
	*/
	auto allocate_device_physical_memory_for_resource(const VkMemoryRequirements &memory_requirements,
													  const VkMemoryPropertyFlags &required_flags = 0,
													  bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		return allocate_device_memory_for_resource(memory_requirements, required_flags, preferred_flags, private_memory);
	}

	/**
	*	@brief	Attempts to allocate host visible memory and bind that memory to the resource.
	*			Thread safe.
	*			
	*	@throws device_memory_exception	On heap allocation failure
	*	@throws device_memory_no_supported_heap_exception	If no device heap conforming to requirements and flags found
	*	@throws vk_memory_allocation_failed_exception	On device memory allocation failure
	*	@throws vk_exception			On other Vulkan errors
	*
	*	@param memory_requirements	Memory requirements
	*	@param required_flags	Required memory flags.
	*	@param private_memory	If set to true, the allocation will create a pivate chunk only for this allocation, i.e. this chunk will not
	*							be shared with other alocation.
	*/
	auto allocate_host_visible_memory_for_resource(const VkMemoryRequirements &memory_requirements,
												   const VkMemoryPropertyFlags &required_flags = 0,
												   bool private_memory = false) const {
		VkMemoryPropertyFlags preferred_flags = required_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		return allocate_device_memory_for_resource(memory_requirements, required_flags, preferred_flags, private_memory);
	}

	/**
	*	@brief	Returns total memory, of type conforming to provided flags, available to device.
	*			Thread safe.
	*	
	*	@param	flags	Memory type flags
	*/
	auto get_total_device_memory(const VkMemoryPropertyFlags &flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get_physical_device_descriptor().memory_properties;

		std::unordered_set<std::uint32_t> conforming_heaps;
		for (int i=0;i<properties.memoryTypeCount;++i) {
			auto &type = properties.memoryTypes[i];
			if ((type.propertyFlags & flags) == flags)
				conforming_heaps.insert(type.heapIndex);
		}

		std::uint64_t total_device_memory = 0;
		for (auto &heap_idx : conforming_heaps) 
			total_device_memory += properties.memoryHeaps[heap_idx].size;

		return total_device_memory;
	}

	/**
	*	@brief	Returns the total commited device memory of specified type.
	*			Thread safe.
	*
	*	@param	type	Memory type
	*/
	auto get_total_commited_memory_of_type(const memory_type_t &type) const {
		std::uint64_t total_commited_memory = 0;

		assert(type < heaps.size());
		auto &heap = *heaps[type];

		{
			std::unique_lock<std::mutex> lock(heap.m);

			for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it)
				total_commited_memory += (*it)->get_heap_size();
			for (auto it = heap.private_chunks.begin(); it != heap.private_chunks.end(); ++it)
				total_commited_memory += (*it)->get_heap_size();
		}

		return total_commited_memory;
	}

	/**
	*	@brief	Returns the total commited device memory, by all heaps of type conforming to provided flags.
	*			Thread safe.
	*
	*	@param	flags	Memory type flags
	*/
	auto get_total_commited_memory(const VkMemoryPropertyFlags &flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get_physical_device_descriptor().memory_properties;
		std::uint64_t total_commited_memory = 0;

		for (memory_type_t type = 0; type < memory_types; ++type) {
			auto &mem_type = properties.memoryTypes[type];
			if ((mem_type.propertyFlags & flags) == flags)
				total_commited_memory += get_total_commited_memory_of_type(type);
		}

		return total_commited_memory;
	}

	/**
	*	@brief	Returns the total allocated device memory of specified type.
	*			Thread safe.
	*
	*	@param	type	Memory type
	*/
	auto get_total_allocated_memory_of_type(const memory_type_t &type) const {
		std::uint64_t total_allocated_memory = 0;

		assert(type < heaps.size());
		auto &heap = *heaps[type];

		{
			std::unique_lock<std::mutex> lock(heap.m);

			for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it)
				total_allocated_memory += (*it)->get_allocated_bytes();
			for (auto it = heap.private_chunks.begin(); it != heap.private_chunks.end(); ++it)
				total_allocated_memory += (*it)->get_allocated_bytes();
		}

		return total_allocated_memory;
	}

	/**
	*	@brief	Returns the total allocated device memory, by all heaps of type conforming to provided flags.
	*			Thread safe.
	*
	*	@param	flags	Memory type flags
	*/
	auto get_total_allocated_memory(const VkMemoryPropertyFlags &flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get_physical_device_descriptor().memory_properties;
		std::uint64_t total_allocated_memory = 0;

		for (memory_type_t type = 0; type < memory_types; ++type) {
			auto &mem_type = properties.memoryTypes[type];
			if ((mem_type.propertyFlags & flags) == flags)
				total_allocated_memory += get_total_allocated_memory_of_type(type);
		}

		return total_allocated_memory;
	}
};

}
}
