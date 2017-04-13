//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_sampler.hpp>
#include <vk_image_view.hpp>
#include <buffer_view.hpp>

#include <vector>

namespace StE {
namespace GL {

struct vk_descriptor_set_write_image {
	VkImageView image{ VK_NULL_HANDLE };
	VkImageLayout image_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
	VkSampler sampler{ VK_NULL_HANDLE };

	vk_descriptor_set_write_image() = default;
	vk_descriptor_set_write_image(VkImageView image_view,
								  VkImageLayout image_layout)
		: image(image_view), image_layout(image_layout) {}
	vk_descriptor_set_write_image(VkImageView image_view,
								  VkImageLayout image_layout, 
								  const vk_sampler &sampler)
		: image(image_view), 
		image_layout(image_layout), 
		sampler(sampler) {}
	vk_descriptor_set_write_image(const vk_sampler &sampler)
		: sampler(sampler) {}

	operator VkDescriptorImageInfo() const {
		return { sampler, image, image_layout };
	}
};

struct vk_descriptor_set_write_buffer {
	VkBuffer buffer{ VK_NULL_HANDLE };
	std::uint64_t buffer_offset{ 0 };
	std::uint64_t buffer_range{ 0 };

	vk_descriptor_set_write_buffer(const buffer_view &buffer)
		: buffer(buffer->get_buffer_handle()), 
		buffer_offset(buffer.offset_bytes()), 
		buffer_range(buffer.range_bytes()) {}

	operator VkDescriptorBufferInfo() const {
		return { buffer, buffer_offset, buffer_range };
	}
};

class vk_descriptor_set_write_resource {
	friend class vk_descriptor_set;

private:
	VkDescriptorType type;
	std::uint32_t binding_index;
	std::uint32_t array_element;

	std::vector<vk_descriptor_set_write_image> image_writes;
	std::vector<vk_descriptor_set_write_buffer> buffer_writes;

public:
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const std::vector<vk_descriptor_set_write_image> &image_writes)
		: type(type), 
		binding_index(binding_index), 
		array_element(array_element),
		image_writes(image_writes)
	{}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const vk_descriptor_set_write_image &image_write)
		: type(type), 
		binding_index(binding_index), 
		array_element(array_element),
		image_writes({ image_write })
	{}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const std::vector<vk_descriptor_set_write_buffer> &buffer_writes)
		: type(type), 
		binding_index(binding_index), 
		array_element(array_element),
		buffer_writes(buffer_writes)
	{}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const vk_descriptor_set_write_buffer &buffer_write)
		: type(type), 
		binding_index(binding_index), 
		array_element(array_element),
		buffer_writes({ buffer_write })
	{}

	vk_descriptor_set_write_resource(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource &operator=(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource(const vk_descriptor_set_write_resource &) = default;
	vk_descriptor_set_write_resource &operator=(const vk_descriptor_set_write_resource &) = default;
};

}
}
