//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_exception.hpp>
#include <vk_device_memory.hpp>
#include <vk_mapped_memory_range.hpp>

#include <functional>
#include <vector>

namespace StE {
namespace GL {

template <typename T>
class vk_mmap {
private:
	using map_pointer = T*;

private:
	vk_device_memory &memory;
	map_pointer ptr;
	std::uint64_t offset;
	std::uint64_t count;

private:
	auto vk_mapped_memory_ranges(const std::vector<vk_mapped_memory_range> &ranges) const {
		std::vector<VkMappedMemoryRange> mapped_ranges;
		mapped_ranges.resize(ranges.size());
		for (int i = 0; i < ranges.size(); ++i) {
			auto &r = ranges[i];

			VkMappedMemoryRange s = {};
			s.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			s.pNext = nullptr;
			s.size = r.size_elements * sizeof(T);
			s.offset = r.offset_elements * sizeof(T);
			s.memory = memory;

			mapped_ranges[i] = s;
		}

		return mapped_ranges;
	}

public:
	vk_mmap(vk_device_memory &memory, std::uint64_t offset, std::uint64_t count, map_pointer ptr)
		: memory(memory), ptr(ptr), offset(offset), count(count) {}
	~vk_mmap() noexcept { munmap(); }

	vk_mmap(vk_mmap &&) = default;
	vk_mmap &operator=(vk_mmap &&) = default;
	vk_mmap(const vk_mmap &) = delete;
	vk_mmap &operator=(const vk_mmap &) = delete;

	void munmap() const {
		memory.munmap();
	}

	/**
	 *	@brief	Flushes mapped memory regions to make writes performed by the host visible to the device
	 */
	void flush_ranges(const std::vector<vk_mapped_memory_range> &ranges) const {
		std::vector<VkMappedMemoryRange> mapped_ranges = vk_mapped_memory_ranges(ranges);

		vk_result res = vkFlushMappedMemoryRanges(memory.get_creating_device(), 
												  mapped_ranges.size(),
												  mapped_ranges.data());
		if (!res) {
			throw vk_exception(res);
		}
	}

	/**
	*	@brief	Invalidates mapped memory ranges to make changes done by the device visible to the host
	*/
	void invalidate_ranges(const std::vector<vk_mapped_memory_range> &ranges) const {
		std::vector<VkMappedMemoryRange> mapped_ranges = vk_mapped_memory_ranges(ranges);

		vk_result res = vkInvalidateMappedMemoryRanges(memory.get_creating_device(),
													   mapped_ranges.size(),
													   mapped_ranges.data());
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_mapped_ptr() { return ptr; }
	auto& get_mapped_ptr() const { return ptr; }
	auto get_size_bytes() const { return count * sizeof(T); }
	auto get_offset() const { return offset; }

	auto* operator->() { return ptr; }
	auto* operator->() const { return ptr; }
	auto& operator*() { return *ptr; }
	auto& operator*() const { return *ptr; }
	auto& operator[](int idx) { return ptr[idx]; }
	auto& operator[](int idx) const { return ptr[idx]; }
	operator map_pointer() { return get_mapped_ptr(); }
	operator map_pointer() const { return get_mapped_ptr(); }
};

}
}
