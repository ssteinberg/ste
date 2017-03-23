//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class vk_buffer_base : public allow_class_decay<vk_buffer_base, VkBuffer> {
private:
	static constexpr auto sparse_buffer_flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

protected:
	std::reference_wrapper<const vk_logical_device> device;

private:
	optional<VkBuffer> buffer;
	std::uint64_t bytes;
	std::uint32_t element_size_bytes;
	VkBufferUsageFlags usage;
	bool sparse;

public:
	vk_buffer_base(const vk_logical_device &device,
				   std::uint64_t bytes,
				   std::uint32_t element_size_bytes,
				   const VkBufferUsageFlags &usage,
				   bool sparse)
		: device(device), bytes(bytes), element_size_bytes(element_size_bytes), usage(usage), sparse(sparse)
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
			vkDestroyBuffer(device.get(), *this, nullptr);
			buffer = none;
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return buffer.get(); }

	auto& get_size_bytes() const { return bytes; }
	auto& get_element_size_bytes() const { return element_size_bytes; }
	auto& get_usage() const { return usage; }
	bool is_sparse() const { return sparse; }
};

}
}
