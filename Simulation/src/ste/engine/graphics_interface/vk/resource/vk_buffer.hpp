//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_ext_debug_marker.hpp>
#include <vk_result.hpp>
#include <vk_host_allocator.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_buffer : public allow_type_decay<vk_buffer<host_allocator>, VkBuffer> {
private:
	static constexpr auto sparse_buffer_flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

protected:
	alias<const vk_logical_device<host_allocator>> device;

private:
	optional<VkBuffer> buffer;
	VkBufferUsageFlags usage;

public:
	vk_buffer(const vk_logical_device<host_allocator> &device,
			  byte_t bytes,
			  const VkBufferUsageFlags &usage,
			  bool sparse,
			  const char *name)
		: device(device), usage(usage)
	{
		VkBuffer buffer;

		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = sparse ? sparse_buffer_flags : 0;
		create_info.size = static_cast<std::size_t>(bytes);
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.usage = usage;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;

		const vk_result res = vkCreateBuffer(device, &create_info, &host_allocator::allocation_callbacks(), &buffer);
		if (!res) {
			throw vk_exception(res);
		}

		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										buffer,
										VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
										name);

		this->buffer = buffer;
	}
	virtual ~vk_buffer() noexcept { destroy_buffer(); }

	vk_buffer(vk_buffer &&) = default;
	vk_buffer& operator=(vk_buffer &&o) noexcept {
		destroy_buffer();

		buffer = std::move(o.buffer);
		device = std::move(o.device);
		usage = o.usage;

		return *this;
	}
	vk_buffer(const vk_buffer &) = delete;
	vk_buffer& operator=(const vk_buffer &) = delete;

	void destroy_buffer() {
		if (buffer) {
			vkDestroyBuffer(device.get(), *this, &host_allocator::allocation_callbacks());
			buffer = none;
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return buffer.get(); }

	auto& get_usage() const { return usage; }
	byte_t get_size_bytes() const { return get_elements_count() * get_element_size_bytes(); };

	virtual std::uint64_t get_elements_count() const = 0;
	virtual byte_t get_element_size_bytes() const = 0;
	virtual bool is_sparse() const = 0;
};

}

}
}
