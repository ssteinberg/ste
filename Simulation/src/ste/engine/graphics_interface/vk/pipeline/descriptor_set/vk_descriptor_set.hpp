//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_write_resource.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_descriptor_set {
	friend class vk_descriptor_pool;

private:
	optional<VkDescriptorSet> set;
	const vk_descriptor_pool &pool;
	const vk_logical_device &device;

private:
	vk_descriptor_set(VkDescriptorSet set,
					   const vk_logical_device &device,
					   const vk_descriptor_pool &pool)
		: set(set), pool(pool), device(device)
	{}

public:
	~vk_descriptor_set() noexcept { free(); }

	vk_descriptor_set(vk_descriptor_set &&) = default;
	vk_descriptor_set &operator=(vk_descriptor_set &&) = default;
	vk_descriptor_set(const vk_descriptor_set &) = delete;
	vk_descriptor_set &operator=(const vk_descriptor_set &) = delete;

	void free();

	void write(const std::vector<vk_descriptor_set_write_resource> &writes) {
		std::vector<VkWriteDescriptorSet> writes_descriptors(writes.size());
		std::vector<VkDescriptorImageInfo> writes_descriptors_images(writes.size());
		std::vector<VkDescriptorBufferInfo> writes_descriptors_buffers(writes.size());

		for (int i = 0; i < writes.size(); ++i) {
			auto& w = *(writes.begin() + i);

			VkDescriptorImageInfo image_info = {};
			image_info.imageLayout = w.image_layout;
			image_info.imageView = w.image;
			image_info.sampler = w.sampler;
			writes_descriptors_images[i] = image_info;

			VkDescriptorBufferInfo buffer_info = {};
			buffer_info.buffer = w.buffer;
			buffer_info.offset = w.buffer_offset;
			buffer_info.range = w.buffer_range;
			writes_descriptors_buffers[i] = buffer_info;

			VkWriteDescriptorSet d = {};
			d.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			d.pNext = nullptr;
			d.dstSet = *this;
			d.dstBinding = w.binding_index;
			d.dstArrayElement = w.array_element;
			d.descriptorCount = w.count;
			d.descriptorType = w.type;
			d.pImageInfo = &writes_descriptors_images[i];
			d.pBufferInfo = &writes_descriptors_buffers[i];
			d.pTexelBufferView = nullptr;
			writes_descriptors[i] = d;
		}

		vkUpdateDescriptorSets(device,
							   writes_descriptors.size(),
							   writes_descriptors.data(),
							   0,
							   nullptr);
	}

	auto& get_set() const { return set.get(); }

	operator VkDescriptorSet() const { return get_set(); }
};

}
}
