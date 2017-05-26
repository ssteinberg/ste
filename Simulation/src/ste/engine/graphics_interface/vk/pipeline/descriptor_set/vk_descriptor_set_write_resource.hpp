//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_handle.hpp>

#include <vk_sampler.hpp>
#include <vk_image_view.hpp>
#include <buffer_view.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace vk {

struct vk_descriptor_set_write_image {
	VkImageView image{ vk_null_handle };
	VkImageLayout image_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
	VkSampler sampler{ vk_null_handle };

	vk_descriptor_set_write_image() = default;
	vk_descriptor_set_write_image(VkImageView image_view,
								  VkImageLayout image_layout)
		: image(image_view), image_layout(image_layout) 
	{}
	template <typename A>
	vk_descriptor_set_write_image(VkImageView image_view,
								  VkImageLayout image_layout,
								  const vk_sampler<A> &sampler)
		: image(image_view),
		image_layout(image_layout),
		sampler(sampler) 
	{}
	template <typename A>
	vk_descriptor_set_write_image(const vk_sampler<A> &sampler)
		: sampler(sampler) 
	{}

	operator VkDescriptorImageInfo() const {
		return { sampler, image, image_layout };
	}
};

struct vk_descriptor_set_write_buffer {
	VkBuffer buffer{ vk_null_handle };
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
	template <typename>
	friend class vk_descriptor_set;

private:
	VkDescriptorType type;
	std::uint32_t binding_index;
	std::uint32_t array_element;

	lib::vector<vk_descriptor_set_write_image> image_writes;
	lib::vector<vk_descriptor_set_write_buffer> buffer_writes;

private:
	static bool is_image_write_descriptor(const VkDescriptorType &type) {
		return type == VK_DESCRIPTOR_TYPE_SAMPLER ||
			type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
			type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
			type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}

public:
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const lib::vector<vk_descriptor_set_write_image> &image_writes)
		: type(type),
		binding_index(binding_index),
		array_element(array_element),
		image_writes(image_writes)
	{
		assert(is_image_write_descriptor(type));
	}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const vk_descriptor_set_write_image &image_write)
		: type(type),
		binding_index(binding_index),
		array_element(array_element),
		image_writes({ image_write })
	{
		assert(is_image_write_descriptor(type));
	}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const lib::vector<vk_descriptor_set_write_buffer> &buffer_writes)
		: type(type),
		binding_index(binding_index),
		array_element(array_element),
		buffer_writes(buffer_writes)
	{
		assert(!is_image_write_descriptor(type));
	}
	vk_descriptor_set_write_resource(const VkDescriptorType &type,
									 std::uint32_t binding_index,
									 std::uint32_t array_element,
									 const vk_descriptor_set_write_buffer &buffer_write)
		: type(type),
		binding_index(binding_index),
		array_element(array_element),
		buffer_writes({ buffer_write })
	{
		assert(!is_image_write_descriptor(type));
	}

	vk_descriptor_set_write_resource(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource &operator=(vk_descriptor_set_write_resource &&) = default;
	vk_descriptor_set_write_resource(const vk_descriptor_set_write_resource &) = default;
	vk_descriptor_set_write_resource &operator=(const vk_descriptor_set_write_resource &) = default;

	auto get_type() const { return type; }
	auto get_binding_index() const { return binding_index; }
	auto get_array_element() const { return array_element; }
	auto get_count() const { return static_cast<std::uint32_t>(std::max(image_writes.size(), buffer_writes.size())); }

	auto& get_image_writes() const { return image_writes; };
	auto& get_buffer_writes() const { return buffer_writes; };
};

}

}
}
