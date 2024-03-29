//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_write_resource.hpp>
#include <vk_host_allocator.hpp>
#include <vk_descriptor_set_copy_resources.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename>
class vk_descriptor_pool;

template <typename host_allocator = vk_host_allocator<>>
class vk_descriptor_set : public allow_type_decay<vk_descriptor_set<host_allocator>, VkDescriptorSet> {
	friend vk_descriptor_pool<host_allocator>;

private:
	alias<const vk_logical_device<host_allocator>> device;

	optional<VkDescriptorSet> set;
	VkDescriptorPool pool;
	bool pool_allows_freeing_individual_sets;

private:
	vk_descriptor_set(const vk_logical_device<host_allocator> &device,
					  VkDescriptorSet set,
					  VkDescriptorPool pool,
					  bool pool_allows_freeing_individual_sets)
		: device(device), set(set), pool(pool), pool_allows_freeing_individual_sets(pool_allows_freeing_individual_sets)
	{}

public:
	~vk_descriptor_set() noexcept { free(); }

	vk_descriptor_set(vk_descriptor_set &&) = default;
	vk_descriptor_set &operator=(vk_descriptor_set &&) = default;
	vk_descriptor_set(const vk_descriptor_set &) = delete;
	vk_descriptor_set &operator=(const vk_descriptor_set &) = delete;

	void free() {
		if (set && pool_allows_freeing_individual_sets) {
			vkFreeDescriptorSets(device.get(), pool, 1, &set.get());
			set = none;
		}
	}

	void update(const lib::vector<vk_descriptor_set_write_resource> &writes,
				const lib::vector<vk_descriptor_set_copy_resources> &copies) {
		lib::vector<VkWriteDescriptorSet> writes_descriptors;
		lib::vector<VkDescriptorImageInfo> writes_descriptors_images;
		lib::vector<VkDescriptorBufferInfo> writes_descriptors_buffers;
		writes_descriptors.reserve(writes.size());

		std::size_t images_count = 0;
		std::size_t buffer_count = 0;
		for (auto &w : writes) {
			images_count += w.image_writes.size();
			buffer_count += w.buffer_writes.size();
		}
		writes_descriptors_images.reserve(images_count);
		writes_descriptors_buffers.reserve(buffer_count);

		for (auto &w : writes) {
			std::uint32_t count;
			const VkDescriptorImageInfo *image_info_ptr = nullptr;
			const VkDescriptorBufferInfo *buffer_info_ptr = nullptr;

			assert((w.image_writes.size() > 0) ^ (w.buffer_writes.size() > 0));

			if (w.image_writes.size()) {
				count = static_cast<std::uint32_t>(w.image_writes.size());
				for (auto &e : w.image_writes)
					writes_descriptors_images.push_back(e);
				image_info_ptr = writes_descriptors_images.data() + writes_descriptors_images.size() - count;
			}
			else {
				count = static_cast<std::uint32_t>(w.buffer_writes.size());
				for (auto &e : w.buffer_writes)
					writes_descriptors_buffers.push_back(e);
				buffer_info_ptr = writes_descriptors_buffers.data() + writes_descriptors_buffers.size() - count;
			}

			VkWriteDescriptorSet d = {};
			d.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			d.pNext = nullptr;
			d.descriptorType = w.type;
			d.pImageInfo = image_info_ptr;
			d.pBufferInfo = buffer_info_ptr;
			d.pTexelBufferView = nullptr;
			d.dstSet = *this;
			d.dstBinding = w.binding_index;
			d.dstArrayElement = w.array_element;
			d.descriptorCount = count;
			writes_descriptors.push_back(d);
		}

		lib::vector<VkCopyDescriptorSet> copy_descriptors;
		copy_descriptors.reserve(copies.size());
		for (auto &c : copies) {
			VkCopyDescriptorSet d = {};
			d.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
			d.pNext = nullptr;
			d.srcSet = c.src;
			d.srcBinding = c.src_binding_index;
			d.srcArrayElement = c.src_array_element;
			d.dstSet = *this;
			d.dstBinding = c.dst_binding_index;
			d.dstArrayElement = c.dst_array_element;
			d.descriptorCount = c.count;

			copy_descriptors.push_back(d);
		}

		vkUpdateDescriptorSets(device.get(),
							   static_cast<std::uint32_t>(writes_descriptors.size()),
							   writes_descriptors.data(),
							   static_cast<std::uint32_t>(copy_descriptors.size()),
							   copy_descriptors.data());
	}
	void write(const lib::vector<vk_descriptor_set_write_resource> &writes) {
		update(writes, {});
	}
	void write(const vk_descriptor_set_write_resource &write) {
		update({ write }, {});
	}
	void copy(const lib::vector<vk_descriptor_set_copy_resources> &copies) {
		update({}, copies);
	}
	void copy(const vk_descriptor_set_copy_resources &copy) {
		update({}, { copy });
	}

	auto& get() const { return set.get(); }
};

}

}
}
