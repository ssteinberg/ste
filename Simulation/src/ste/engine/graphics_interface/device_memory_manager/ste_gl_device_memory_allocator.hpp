//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_memory_exception.hpp>
#include <vk_handle.hpp>
#include <vk_logical_device.hpp>
#include <vk_device_memory.hpp>
#include <device_memory_heap.hpp>

#include <memory_properties_flags.hpp>
#include <memory_requirements.hpp>

#include <mutex>
#include <array>
#include <lib/unordered_map.hpp>
#include <lib/unordered_set.hpp>
#include <lib/aligned_padded_ptr.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class ste_gl_device_memory_allocator : public unique_device_ptr_allocator {
private:
	using chunk_t = device_memory_heap;
	using chunks_t = lib::unordered_map<std::uint64_t, chunk_t>;
	using memory_type_t = std::uint32_t;

	struct heap_t {
		chunks_t chunks;
		chunks_t private_chunks;
		lib::aligned_padded_ptr<std::mutex> m;
	};

	static constexpr memory_type_t memory_types = 32;

	using heaps_t = std::array<heap_t, memory_types>;

	friend class chunk_t::allocation_type;

public:
	// Allocate 256MB chunks by default
	static constexpr std::uint64_t default_minimal_allocation_size_bytes = 256 * 1024 * 1024;

	using allocation_t = chunk_t::allocation_type;

private:
	alias<const vk::vk_logical_device<>> device;
	mutable heaps_t heaps;
	std::uint64_t minimal_allocation_size_bytes;

private:
	static bool memory_type_matches_requirements(const memory_type_t &type,
												 const memory_requirements &memory_requirements) {
		return !!(memory_requirements.type_bits & (1 << type));
	}

	static memory_type_t find_memory_type_for(const vk::vk_logical_device<> &device,
											  const memory_requirements &memory_requirements,
											  const memory_properties_flags &required_flags,
											  const memory_properties_flags &preferred_flags) {
		const vk::vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();
		int fallback_memory_type = -1;

		// Try to find a heap matching the memory requirments and preferred flags.
		// If none found fallback to a heap matching the requirements and the required flags.
		for (memory_type_t type = 0; type < memory_types; ++type) {
			if (!memory_type_matches_requirements(type, memory_requirements))
				continue;

			auto &heap = physical_device.memory_properties.memoryTypes[type];
			if ((static_cast<memory_properties_flags>(heap.propertyFlags) & preferred_flags) == preferred_flags)
				return type;
			if (fallback_memory_type == -1 &&
				(static_cast<memory_properties_flags>(heap.propertyFlags) & required_flags) == required_flags)
				fallback_memory_type = type;
		}

		if (fallback_memory_type != -1)
			return fallback_memory_type;

		// No heap with requested flags found
		throw device_memory_no_supported_heap_exception();
	}

	void deallocate(const allocation_t &ptr) const override final {
		auto memory_type = ptr.get_memory_type();

		assert(memory_type < heaps.size());
		auto &heap = heaps[memory_type];

		{
			std::unique_lock<std::mutex> lock(*heap.m);

			chunks_t &chunks = ptr.is_private_allocation() ?
				heap.private_chunks :
				heap.chunks;
			auto it = chunks.find(ptr.get_heap_tag());
			
			assert(it != chunks.end());

			it->second.deallocate(ptr);
			if (it->second.get_allocated_bytes() == 0) {
				chunks.erase(it);
			}
		}
	}

	static auto allocate_from_heap(heap_t &heap, 
								   std::uint64_t size, 
								   std::uint64_t alignment) {
		allocation_t allocation;
		for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it) {
			if (!allocation)
				allocation = it->second.allocate(size, alignment, false);

			// Remove empty heaps
			if (it->second.get_allocated_bytes() == 0)
				it = heap.chunks.erase(it);
		}

		return allocation;
	}

	auto commit_device_memory_heap_for_memory_type(const memory_type_t &memory_type,
												   std::uint64_t minimal_size,
												   std::uint64_t alignment,
												   bool dedicated_allocation) const {
		// Calculate chunk size
		auto chunk_size = dedicated_allocation ?
			minimal_size :
			std::max(minimal_size, minimal_allocation_size_bytes);
		// Align it
		chunk_size = chunk_t::align(chunk_size, alignment);

		// Create the device memory object
		return vk::vk_device_memory<>(device, chunk_size, memory_type);
	}

	auto allocate(std::uint64_t size,
				  const memory_requirements &memory_requirements,
				  const memory_properties_flags &required_flags,
				  const memory_properties_flags &preferred_flags) const {
		auto memory_type = find_memory_type_for(device, 
												memory_requirements,
												required_flags, 
												preferred_flags | required_flags);
		bool dedicated_allocation = memory_requirements.dedicated != memory_dedicated_allocation_flag::suballocate;
		auto alignment = memory_requirements.alignment;
		allocation_t allocation;

		assert(memory_type < heaps.size());
		auto &heap = heaps[memory_type];

		{
			std::unique_lock<std::mutex> lock(*heap.m);

			// Prune private chunks
			for (auto it = heap.private_chunks.begin(); it != heap.private_chunks.end(); ++it) {
				if (it->second.get_allocated_bytes() == 0)
					it = heap.private_chunks.erase(it);
			}

			if (!dedicated_allocation && !heap.chunks.empty()) {
				// Try to allocate memory on one of the existing chunks
				allocation = allocate_from_heap(heap, 
												size, 
												alignment);
			}

			// If failed allocating on existing chunks, or no existing chunks available,
			// create a new chunk.
			if (!allocation) {
				chunks_t &chunks = dedicated_allocation ?
					heap.private_chunks :
					heap.chunks;

				// Commit device memory
				auto memory_object = commit_device_memory_heap_for_memory_type(memory_type, 
																			   size, 
																			   alignment, 
																			   dedicated_allocation);
				// And use it to create a new chunk
				auto ret = chunks.emplace(std::piecewise_construct,
										  std::forward_as_tuple(static_cast<std::uint64_t>(vk::vk_handle(memory_object))),
										  std::forward_as_tuple(this,
																memory_type,
																std::move(memory_object)));
				assert(ret.second);
				// Allocate from that chunk
				allocation = ret.first->second.allocate(size, alignment, dedicated_allocation);
			}
		}

		assert(allocation);
		return allocation;
	}

public:
	ste_gl_device_memory_allocator(const vk::vk_logical_device<> &device,
								   std::uint64_t minimal_allocation_size_bytes = default_minimal_allocation_size_bytes)
		: device(device), 
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
								const memory_requirements &memory_requirements,
								const memory_properties_flags &required_flags,
								const memory_properties_flags &preferred_flags) const {
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags);
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
	*/
	auto allocate_device_physical_memory(std::uint64_t size,
										 const memory_requirements &memory_requirements,
										 const memory_properties_flags &required_flags = memory_properties_flags::none) const {
		const memory_properties_flags preferred_flags = required_flags | memory_properties_flags::device_local;
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags);
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
	*/
	auto allocate_host_visible_memory(std::uint64_t size,
									  const memory_requirements &memory_requirements,
									  const memory_properties_flags &required_flags = memory_properties_flags::none) const {
		const memory_properties_flags preferred_flags = required_flags | memory_properties_flags::host_visible;
		return allocate(size,
						memory_requirements,
						required_flags,
						preferred_flags);
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
	*/
	auto allocate_device_memory_for_resource(const memory_requirements &memory_requirements,
											 const memory_properties_flags &required_flags,
											 const memory_properties_flags &preferred_flags) const {
		auto allocation = allocate(memory_requirements.size,
								   memory_requirements,
								   required_flags,
								   preferred_flags);

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
	*/
	auto allocate_device_physical_memory_for_resource(const memory_requirements &memory_requirements,
													  const memory_properties_flags &required_flags = memory_properties_flags::none) const {
		const memory_properties_flags preferred_flags = required_flags | memory_properties_flags::device_local;
		return allocate_device_memory_for_resource(memory_requirements, 
												   required_flags, 
												   preferred_flags);
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
	*/
	auto allocate_host_visible_memory_for_resource(const memory_requirements &memory_requirements,
												   const memory_properties_flags &required_flags = memory_properties_flags::none) const {
		const memory_properties_flags preferred_flags = required_flags | memory_properties_flags::host_visible;
		return allocate_device_memory_for_resource(memory_requirements, 
												   required_flags, 
												   preferred_flags);
	}

	/**
	*	@brief	Returns total memory, of type conforming to provided flags, available to device.
	*			Thread safe.
	*	
	*	@param	flags	Memory type flags
	*/
	auto get_total_device_memory(const memory_properties_flags &flags = memory_properties_flags::device_local) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get().get_physical_device_descriptor().memory_properties;

		lib::unordered_set<std::uint32_t> conforming_heaps;
		for (std::uint32_t i=0;i<properties.memoryTypeCount;++i) {
			auto &type = properties.memoryTypes[i];
			if ((static_cast<memory_properties_flags>(type.propertyFlags) & flags) == flags)
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
		auto &heap = heaps[type];

		{
			std::unique_lock<std::mutex> lock(*heap.m);

			for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it)
				total_commited_memory += it->second.get_heap_size();
			for (auto it = heap.private_chunks.begin(); it != heap.private_chunks.end(); ++it)
				total_commited_memory += it->second.get_heap_size();
		}

		return total_commited_memory;
	}

	/**
	*	@brief	Returns the total commited device memory, by all heaps of type conforming to provided flags.
	*			Thread safe.
	*
	*	@param	flags	Memory type flags
	*/
	auto get_total_commited_memory(const memory_properties_flags &flags = memory_properties_flags::device_local) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get().get_physical_device_descriptor().memory_properties;
		std::uint64_t total_commited_memory = 0;

		for (memory_type_t type = 0; type < memory_types; ++type) {
			auto &mem_type = properties.memoryTypes[type];
			if ((static_cast<memory_properties_flags>(mem_type.propertyFlags) & flags) == flags)
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
		auto &heap = heaps[type];

		{
			std::unique_lock<std::mutex> lock(*heap.m);

			for (auto it = heap.chunks.begin(); it != heap.chunks.end(); ++it)
				total_allocated_memory += it->second.get_allocated_bytes();
			for (auto it = heap.private_chunks.begin(); it != heap.private_chunks.end(); ++it)
				total_allocated_memory += it->second.get_allocated_bytes();
		}

		return total_allocated_memory;
	}

	/**
	*	@brief	Returns the total allocated device memory, by all heaps of type conforming to provided flags.
	*			Thread safe.
	*
	*	@param	flags	Memory type flags
	*/
	auto get_total_allocated_memory(const memory_properties_flags &flags = memory_properties_flags::device_local) const {
		const VkPhysicalDeviceMemoryProperties &properties = device.get().get_physical_device_descriptor().memory_properties;
		std::uint64_t total_allocated_memory = 0;

		for (memory_type_t type = 0; type < memory_types; ++type) {
			auto &mem_type = properties.memoryTypes[type];
			if ((static_cast<memory_properties_flags>(mem_type.propertyFlags) & flags) == flags)
				total_allocated_memory += get_total_allocated_memory_of_type(type);
		}

		return total_allocated_memory;
	}
};

}
}
