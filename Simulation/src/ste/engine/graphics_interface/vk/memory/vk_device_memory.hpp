//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_physical_device_descriptor.hpp>
#include <vk_logical_device.hpp>
#include <vk_memory_exception.hpp>
#include <vk_mmap.hpp>

#include <memory>

namespace StE {
namespace GL {

class vk_device_memory {
private:
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	const vk_logical_device &device;
	std::uint64_t size;

	std::unique_ptr<vk_mmap_type_eraser> mapped_memory{ nullptr };

public:
	static auto allocate_memory(const vk_logical_device &device, 
								std::uint64_t size, 
								const VkMemoryPropertyFlags &required_flags) {
		const vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();

		// Try to find a heap that matches exactly
		for (int i=0; i<physical_device.memory_properties.memoryTypeCount; ++i) {
			auto &heap = physical_device.memory_properties.memoryTypes[i];
			if (heap.propertyFlags == required_flags)
				return vk_device_memory(device, size, heap.heapIndex);
		}

		// Otherwise try to find a heap that satisfies all flags
		for (int i = 0; i<physical_device.memory_properties.memoryTypeCount; ++i) {
			auto &heap = physical_device.memory_properties.memoryTypes[i];
			if ((heap.propertyFlags & required_flags) == required_flags)
				return vk_device_memory(device, size, heap.heapIndex);
		}

		// No heap with requested flags found
		throw vk_memory_no_supported_heap_exception();
	}

	static auto allocate_memory(const vk_logical_device &device,
								std::uint64_t size,
								const VkMemoryRequirements &memory_requirements,
								const VkMemoryPropertyFlags &required_flags,
								const VkMemoryPropertyFlags &preffered_flags) {
		const vk_physical_device_descriptor &physical_device = device.get_physical_device_descriptor();
		int fallback_heap_index = -1;
		size = glm::max(size, memory_requirements.size);

		// Try to find a heap matching the memory requirments and preffered flags.
		// If none found fallback to a heap matching the requirements and the required flags.
		for (int type = 0; type < 32; ++type) {
			if (!(memory_requirements.memoryTypeBits & (1 << type)))
				continue;

			auto &heap = physical_device.memory_properties.memoryTypes[type];
			if ((heap.propertyFlags & preffered_flags) == preffered_flags)
				return vk_device_memory(device, size, heap.heapIndex);
			if (fallback_heap_index == -1 &&
				(heap.propertyFlags & required_flags) == required_flags)
				fallback_heap_index = heap.heapIndex;
		}

		if (fallback_heap_index != -1)
			return vk_device_memory(device, size, fallback_heap_index);

		// No heap with requested flags found
		throw vk_memory_no_supported_heap_exception();
	}

	static auto allocate_device_physical_memory(const vk_logical_device &device, std::uint64_t size) {
		return allocate_memory(device, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	static auto allocate_host_visible_memory(const vk_logical_device &device, std::uint64_t size) {
		return allocate_memory(device, size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

public:
	vk_device_memory(const vk_logical_device &device, std::uint64_t size, int memory_type_index)
		: device(device), size(size)
	{
		VkDeviceMemory memory;

		VkMemoryAllocateInfo info;
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
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}

	auto get_allocation_memory_commitment() const {
		std::uint64_t committed_bytes;
		vkGetDeviceMemoryCommitment(device, memory, &committed_bytes);
		return committed_bytes;
	}

	template <typename T>
	std::unique_ptr<vk_mmap<T>> mmap(std::uint64_t offset, std::uint64_t count) {
		munmap();

		void *pdata = nullptr;
		vk_result res = vkMapMemory(device, memory, offset * sizeof(T), count * sizeof(T), 0, &pdata);
		if (!res) {
			throw vk_exception(res);
		}

		auto mmap = std::make_unique<vk_mmap<T>>(*this, offset, count, reinterpret_cast<T*>(pdata));
		mapped_memory = std::make_unique<vk_mmap_type_eraser>(mmap.get());

		return std::move(mmap);
	}
	void munmap() {
		if (mapped_memory != nullptr) {
			vkUnmapMemory(device, memory);
			mapped_memory = nullptr;
		}
	}

	auto& get_creating_device() const { return device; }
	auto& get_device_memory() const { return memory; }
	auto get_size() const { return size; }

	operator VkDeviceMemory() const { return get_device_memory(); }
};

}
}
