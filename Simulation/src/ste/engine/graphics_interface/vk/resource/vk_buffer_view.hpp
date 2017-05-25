//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_buffer.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_buffer_view {
private:
	optional<VkBufferView> view;
	alias<const vk_logical_device> device;
	std::uint64_t size_bytes;
	VkFormat format;

public:
	vk_buffer_view(const vk_buffer &parent,
				   const VkFormat &format,
				   std::uint64_t offset_bytes,
				   std::uint64_t size_bytes)
		: device(parent.get_creating_device()), size_bytes(size_bytes), format(format)
	{
		VkBufferView view;

		VkBufferViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.buffer = parent;
		create_info.offset = offset_bytes;
		create_info.range = size_bytes;
		create_info.format = format;

		vk_result res = vkCreateBufferView(device.get(), &create_info, nullptr, &view);
		if (!res) {
			throw vk_exception(res);
		}

		this->view = view;
	}
	vk_buffer_view(const vk_buffer &parent,
				   const VkFormat &format,
				   std::uint64_t offset_bytes = 0)
		: vk_buffer_view(parent, format, offset_bytes, parent.get_size_bytes() - offset_bytes)
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
			vkDestroyBufferView(device.get(), view.get(), nullptr);
			view = none;
		}
	}

	auto& get_view() const { return view.get(); }
	auto& get_size_bytes() const { return size_bytes; }
	auto& get_format() const { return format; }

	operator VkBufferView() const { return get_view(); }
};

}

}
}
