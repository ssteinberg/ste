//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

namespace StE {
namespace GL {

template <typename T>
class vk_buffer {
private:
	VkBuffer buffer{ VK_NULL_HANDLE };
	const vk_logical_device &device;
	std::uint64_t size;
	VkBufferUsageFlags usage;
	bool sparse;

public:
	vk_buffer(const vk_logical_device &device,
			  std::uint64_t size,
			  const VkBufferUsageFlags &usage,
			  bool sparse = false) 
		: device(device), size(size), usage(usage), sparse(sparse)
	{
		VkBuffer buffer;

		auto bytes = size * sizeof(T);

		VkBufferCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = sparse ?
			VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT :
			0;
		create_info.size = bytes;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.usage = usage;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;

		vk_result res = vkCreateBuffer(device, &create_info, nullptr, &buffer);
		if (!res) {
			throw vk_exception(res);
		}

		this->buffer = buffer;
	}
	~vk_buffer() noexcept { destroy_buffer(); }

	vk_buffer(vk_buffer &&) = default;
	vk_buffer& operator=(vk_buffer &&) = default;
	vk_buffer(const vk_buffer &) = delete;
	vk_buffer& operator=(const vk_buffer &) = delete;

	void destroy_buffer() {
		if (buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
	}

	auto& get_buffer() const { return buffer; }

	auto& get_creating_device() const { return device; }
	auto& get_size() const { return size; }
	auto& get_usage() const { return usage; }
	auto is_sparse() const { return sparse; }
	
	operator VkBuffer() const { return get_buffer(); }
};

}
}
