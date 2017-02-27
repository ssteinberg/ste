//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_buffer.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

namespace StE {
namespace GL {

class vk_buffer_view {
private:
	VkBufferView view{ VK_NULL_HANDLE };
	const vk_logical_device &device;
	std::uint64_t size_bytes;
	VkFormat format;

public:
	template <typename T>
	vk_buffer_view(const vk_buffer<T> &parent,
				   const VkFormat &format,
				   std::uint64_t offset_bytes,
				   std::uint64_t size_bytes)
		: device(parent.get_creating_device()), size_bytes(size_bytes), format(format)
	{
		VkBufferView view;

		VkBufferViewCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.buffer = parent;
		create_info.offset = offset_bytes;
		create_info.range = size_bytes;
		create_info.format = format;

		vk_result res = vkCreateBufferView(device, &create_info, nullptr, &view);
		if (!res) {
			throw vk_exception(res);
		}

		this->view = view;
	}
	template <typename T>
	vk_buffer_view(const vk_buffer<T> &parent,
				   const VkFormat &format,
				   std::uint64_t offset_bytes = 0) 
		: vk_buffer_view(parent, format, offset_bytes, parent.get_size() * sizeof(T) - offset_bytes)
	{}
	~vk_buffer_view() noexcept { destroy_view(); }

	vk_buffer_view(vk_buffer_view &&) = default;
	vk_buffer_view& operator=(vk_buffer_view &&) = default;
	vk_buffer_view(const vk_buffer_view &) = delete;
	vk_buffer_view& operator=(const vk_buffer_view &) = delete;

	void destroy_view() {
		if (view != VK_NULL_HANDLE) {
			vkDestroyBufferView(device, view, nullptr);
			view = VK_NULL_HANDLE;
		}
	}

	auto& get_view() const { return view; }
	auto& get_size_bytes() const { return size_bytes; }
	auto& get_format() const { return format; }

	operator VkBufferView() const { return get_view(); }
};

}
}
