//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_sampler.hpp>
#include <vk_image_view.hpp>
#include <vk_buffer.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_descriptor_set_write_resource {
	friend class vk_descriptor_set;

private:
	VkDescriptorType type;
	std::uint32_t binding_index;
	std::uint32_t array_element;
	std::uint32_t count;

	VkImageView image{ VK_NULL_HANDLE };
	VkSampler sampler{ VK_NULL_HANDLE };
	VkImageLayout image_layout{ VK_IMAGE_LAYOUT_UNDEFINED };

	VkBuffer buffer{ VK_NULL_HANDLE };
	std::uint64_t buffer_offset{ 0 };
	std::uint64_t buffer_range{ 0 };

public:
	template <vk_image_type T>
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 std::uint32_t count,
									 const vk_image_view<T> *image_view,
									 VkImageLayout image_layout,
									 const vk_sampler *sampler = nullptr)
		: type(type), binding_index(binding_index), array_element(array_element), count(count),
		image(*image_view), sampler(sampler ? *sampler : VK_NULL_HANDLE), image_layout(image_layout)
	{}
	template <typename T>
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 std::uint32_t count,
									 const vk_buffer<T> *buffer,
									 std::uint64_t buffer_range,
									 std::uint64_t buffer_offset = 0)
		: type(type), binding_index(binding_index), array_element(array_element), count(count),
		buffer(*buffer), buffer_offset(buffer_offset), buffer_range(buffer_range)
	{}

	vk_descriptor_set_write_resource(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource &operator=(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource(const vk_descriptor_set_write_resource &) = delete;
	vk_descriptor_set_write_resource &operator=(const vk_descriptor_set_write_resource &) = delete;
};

}
}
