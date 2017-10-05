//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_buffer.hpp>
#include <vk_ext_debug_marker.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>
#include <alias.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_buffer_view : public allow_type_decay<vk_buffer_view<host_allocator>, VkBufferView> {
private:
	optional<VkBufferView> view;
	alias<const vk_logical_device<host_allocator>> device;
	byte_t size_bytes;
	VkFormat format;

public:
	vk_buffer_view(const vk_buffer<host_allocator> &parent,
				   const VkFormat &format,
				   byte_t offset_bytes,
				   byte_t size_bytes,
				   const char *name)
		: device(parent.get_creating_device()), size_bytes(size_bytes), format(format)
	{
		VkBufferView view;

		VkBufferViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.buffer = parent;
		create_info.offset = static_cast<std::size_t>(offset_bytes);
		create_info.range = static_cast<std::size_t>(size_bytes);
		create_info.format = format;

		const vk_result res = vkCreateBufferView(device.get(), &create_info, &host_allocator::allocation_callbacks(), &view);
		if (!res) {
			throw vk_exception(res);
		}


		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										view,
										VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
										name);

		this->view = view;
	}
	vk_buffer_view(const vk_buffer<host_allocator> &parent,
				   const VkFormat &format,
				   byte_t offset_bytes,
				   const char *name)
		: vk_buffer_view(parent, 
						 format, 
						 offset_bytes, 
						 parent.get_size_bytes() - offset_bytes,
						 name)
	{}
	~vk_buffer_view() noexcept { destroy_view(); }

	vk_buffer_view(vk_buffer_view &&) = default;
	vk_buffer_view& operator=(vk_buffer_view &&o) noexcept {
		destroy_view();

		view = std::move(o.view);
		device = std::move(o.device);
		size_bytes = o.size_bytes;
		format = o.format;

		return *this;
	}
	vk_buffer_view(const vk_buffer_view &) = delete;
	vk_buffer_view& operator=(const vk_buffer_view &) = delete;

	void destroy_view() {
		if (view) {
			vkDestroyBufferView(device.get(), view.get(), &host_allocator::allocation_callbacks());
			view = none;
		}
	}

	auto& get() const { return view.get(); }
	auto& get_size_bytes() const { return size_bytes; }
	auto& get_format() const { return format; }
};

}

}
}
