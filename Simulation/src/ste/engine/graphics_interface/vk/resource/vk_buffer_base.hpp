//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_buffer_base {
private:
	static constexpr auto sparse_buffer_flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

protected:
	const vk_logical_device &device;

private:
	optional<VkBuffer> buffer;
	std::uint64_t bytes;
	VkBufferUsageFlags usage;
	bool sparse;

public:
	vk_buffer_base(const vk_logical_device &device,
				   std::uint64_t bytes,
				   const VkBufferUsageFlags &usage,
				   bool sparse)
		: device(device), bytes(bytes), usage(usage), sparse(sparse)
	{
		VkBuffer buffer;

		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = sparse ? sparse_buffer_flags : 0;
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
	virtual ~vk_buffer_base() noexcept { destroy_buffer(); }

	vk_buffer_base(vk_buffer_base &&) = default;
	vk_buffer_base& operator=(vk_buffer_base &&) = default;
	vk_buffer_base(const vk_buffer_base &) = delete;
	vk_buffer_base& operator=(const vk_buffer_base &) = delete;

	void destroy_buffer() {
		if (buffer) {
			vkDestroyBuffer(device, *this, nullptr);
			buffer = none;
		}
	}

	auto& get_creating_device() const { return device; }
	auto& get_buffer() const { return buffer.get(); }

	auto& get_size_bytes() const { return bytes; }
	auto& get_usage() const { return usage; }
	bool is_sparse() const { return sparse; }

	operator VkBuffer() const { return get_buffer(); }
};

}
}
