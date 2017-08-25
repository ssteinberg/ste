//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <vk_logical_device.hpp>
#include <vk_memory_exception.hpp>
#include <vk_mmap.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>

#include <lib/unique_ptr.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_mmap_type_eraser {
private:
	struct ctor {};
	std::function<void()> unmapper;

public:
	vk_mmap_type_eraser(ctor,
						std::function<void()> &&f) : unmapper(std::move(f)) {}
	~vk_mmap_type_eraser() noexcept { unmapper(); }

	template <typename T, typename A>
	static auto create(const vk_mmap<T, A> *m) {
		return lib::allocate_unique<vk_mmap_type_eraser>(ctor(), [m]() {
			m->munmap();
		});
	}

	vk_mmap_type_eraser(vk_mmap_type_eraser &&) = default;
	vk_mmap_type_eraser&operator=(vk_mmap_type_eraser &&) = default;
	vk_mmap_type_eraser(const vk_mmap_type_eraser &) = delete;
	vk_mmap_type_eraser&operator=(const vk_mmap_type_eraser &) = delete;
};

template <typename host_allocator = vk_host_allocator<>>
class vk_device_memory : public allow_type_decay<vk_device_memory<host_allocator>, VkDeviceMemory> {
private:
	optional<VkDeviceMemory> memory;
	alias<const vk_logical_device<host_allocator>> device;
	std::uint64_t size;

	lib::unique_ptr<vk_mmap_type_eraser> mapped_memory{ nullptr };

public:
	vk_device_memory(const vk_logical_device<host_allocator> &device, std::uint64_t size, int memory_type_index)
		: device(device), size(size)
	{
		VkDeviceMemory memory;

		VkMemoryAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.allocationSize = size;
		info.memoryTypeIndex = memory_type_index;
		vk_result res = vkAllocateMemory(device, &info, &host_allocator::allocation_callbacks(), &memory);
		if (!res) {
			throw vk_memory_allocation_failed_exception(res);
		}

		this->memory = memory;
	}
	~vk_device_memory() noexcept { free(); }

	vk_device_memory(vk_device_memory &&) = default;
	vk_device_memory &operator=(vk_device_memory &&) = delete;
	vk_device_memory(const vk_device_memory &) = delete;
	vk_device_memory &operator=(const vk_device_memory &) = delete;

	void free() {
		if (memory) {
			munmap();

			vkFreeMemory(device.get(), *this, &host_allocator::allocation_callbacks());
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
		vkGetDeviceMemoryCommitment(device.get(), *this, &committed_bytes);
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
	auto mmap(std::uint64_t offset, std::uint64_t count) {
		munmap();

		void *pdata = nullptr;
		vk_result res = vkMapMemory(device.get(), *this, offset * sizeof(T), count * sizeof(T), 0, &pdata);
		if (!res) {
			throw vk_exception(res);
		}

		auto mmap = lib::allocate_unique<vk_mmap<T, host_allocator>>(*this, offset, count, reinterpret_cast<T*>(pdata));
		mapped_memory = vk_mmap_type_eraser::create<T>(mmap.get());

		return mmap;
	}
	/**
	*	@brief	Unmap the previously mmaped region. If no region is mapped, call will silently fail.
	*/
	void munmap() {
		if (mapped_memory != nullptr) {
			vkUnmapMemory(device.get(), *this);
			mapped_memory = nullptr;
		}
	}

	/**
	*	@brief	Returns the non-coherent atom size. See mmap().
	*			Guaranteed by the specification to be 256 bytes at most.
	*/
	auto get_non_coherent_atom_size() const {
		return device.get().get_physical_device_descriptor().properties.limits.nonCoherentAtomSize;
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get_device_memory() const { return memory.get(); }
	auto get_size() const { return size; }

	auto& get() const { return get_device_memory(); }
};

}

}
}
