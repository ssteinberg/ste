//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_buffer : public allow_type_decay<vk_buffer, VkBuffer> {
private:
	static constexpr auto sparse_buffer_flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

protected:
	std::reference_wrapper<const vk_logical_device> device;

private:
	optional<VkBuffer> buffer;
	VkBufferUsageFlags usage;

public:
	vk_buffer(const vk_logical_device &device,
				   std::uint64_t bytes,
				   const VkBufferUsageFlags &usage,
				   bool sparse)
		: device(device), usage(usage)
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
	virtual ~vk_buffer() noexcept { destroy_buffer(); }

	vk_buffer(vk_buffer &&) = default;
	vk_buffer& operator=(vk_buffer &&) = default;
	vk_buffer(const vk_buffer &) = delete;
	vk_buffer& operator=(const vk_buffer &) = delete;

	void destroy_buffer() {
		if (buffer) {
			vkDestroyBuffer(device.get(), *this, nullptr);
			buffer = none;
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return buffer.get(); }

	auto& get_usage() const { return usage; }
	std::uint64_t get_size_bytes() const { return get_elements_count() * get_element_size_bytes(); };

	virtual std::uint64_t get_elements_count() const = 0;
	virtual std::uint32_t get_element_size_bytes() const = 0;
	virtual bool is_sparse() const = 0;
};

}

}
}
