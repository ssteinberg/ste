//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_physical_device_descriptor.hpp>
#include <vk_logical_device.hpp>
#include <vk_memory_exception.hpp>

#include <optional.hpp>
#include <allow_class_decay.hpp>

#include <memory>

namespace StE {
namespace GL {

template <typename T>
class vk_mmap;

class vk_mmap_type_eraser {
private:
	std::function<void()> unmapper;

public:
	template <typename T>
	vk_mmap_type_eraser(const vk_mmap<T> *m) : unmapper([m]() {
		m->munmap();
	}) {}
	~vk_mmap_type_eraser() noexcept { unmapper(); }

	vk_mmap_type_eraser(vk_mmap_type_eraser &&) = default;
	vk_mmap_type_eraser&operator=(vk_mmap_type_eraser &&) = default;
	vk_mmap_type_eraser(const vk_mmap_type_eraser &) = delete;
	vk_mmap_type_eraser&operator=(const vk_mmap_type_eraser &) = delete;
};

class vk_device_memory : public allow_class_decay<vk_device_memory, VkDeviceMemory> {
private:
	optional<VkDeviceMemory> memory;
	const vk_logical_device &device;
	std::uint64_t size;

	std::unique_ptr<vk_mmap_type_eraser> mapped_memory{ nullptr };

public:
	vk_device_memory(const vk_logical_device &device, std::uint64_t size, int memory_type_index)
		: device(device), size(size)
	{
		VkDeviceMemory memory;

		VkMemoryAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.allocationSize = size;
		info.memoryTypeIndex = memory_type_index;
		vk_result res = vkAllocateMemory(device, &info, nullptr, &memory);
		if (!res) {
			throw vk_memory_allocation_failed_exception(res);
		}

		this->memory = memory;
	}
	~vk_device_memory() noexcept { free(); }

	vk_device_memory(vk_device_memory &&) = default;
	vk_device_memory &operator=(vk_device_memory &&) = default;
	vk_device_memory(const vk_device_memory &) = delete;
	vk_device_memory &operator=(const vk_device_memory &) = delete;

	void free() {
		if (memory) {
			munmap();

			vkFreeMemory(device, *this, nullptr);
			memory = none;
		}
	}

	/**
	*	@brief	Returns the amount, in bytes, of the device commited memory of lazily-allocated 
	*			(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) memory. 
	*			
	*			It is the callers responsibility to ensure that this memory object was allocated with a lazily-allocated
	*			memory type, otherwise the return value is undefined.
	*/
	auto get_allocation_memory_commitment() const {
		std::uint64_t committed_bytes;
		vkGetDeviceMemoryCommitment(device, *this, &committed_bytes);
		return committed_bytes;
	}

	/**
	*	@brief	Maps the memory object and returns the host addressable virtual address pointer to the mapped region.
	*			Only one region can be mapped at a time.
	*			Unmaps a previously mapped region, if any.
	*			
	*			It is the caller responsibility to ensure correct memory type and alignment:
	*			Device memory must be allocated on a host visible heap (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), otherwise 
	*			call will fail and throw.
	*			If the device memory is non-coherent (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), the mapped region start 
	*			offset and size in bytes should be aligned to non-coherent atom size bounderies (retrieveable 
	*			via get_non_coherent_atom_size()), otherwise call will fail and throw.
	*
	*	@param	offset	Mapped region start offset
	*	@param	count	Mapped region size expressed in count of elements of type T
	*/
	template <typename T>
	std::unique_ptr<vk_mmap<T>> mmap(std::uint64_t offset, std::uint64_t count) {
		munmap();

		void *pdata = nullptr;
		vk_result res = vkMapMemory(device, *this, offset * sizeof(T), count * sizeof(T), 0, &pdata);
		if (!res) {
			throw vk_exception(res);
		}

		auto mmap = std::make_unique<vk_mmap<T>>(*this, offset, count, reinterpret_cast<T*>(pdata));
		mapped_memory = std::make_unique<vk_mmap_type_eraser>(mmap.get());

		return std::move(mmap);
	}
	/**
	*	@brief	Unmap the previously mmaped region. If no region is mapped, call will silently fail.
	*/
	void munmap() {
		if (mapped_memory != nullptr) {
			vkUnmapMemory(device, *this);
			mapped_memory = nullptr;
		}
	}

	/**
	*	@brief	Returns the non-coherent atom size. See mmap().
	*			Guaranteed by the specification to be 256 bytes at most.
	*/
	auto get_non_coherent_atom_size() const { 
		return device.get_physical_device_descriptor().properties.limits.nonCoherentAtomSize;
	}

	auto& get_creating_device() const { return device; }
	auto& get_device_memory() const { return memory.get(); }
	auto get_size() const { return size; }

	auto& get() const { return get_device_memory(); }
};

}
}

#include <vk_mmap.hpp>
