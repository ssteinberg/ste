//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout_binding.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_set_layout.hpp>

#include <vector>
#include <unordered_map>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class vk_descriptor_pool : public allow_class_decay<vk_descriptor_pool, VkDescriptorPool> {
private:
	optional<VkDescriptorPool> pool;
	std::reference_wrapper<const vk_logical_device> device;
	bool allow_free_individual_sets;

public:
	vk_descriptor_pool(const vk_logical_device &device,
					   std::uint32_t max_sets,
					   const std::vector<vk_descriptor_set_layout_binding> &set_layouts,
					   bool allow_free_individual_sets = false) : device(device), allow_free_individual_sets(allow_free_individual_sets) {
		std::unordered_map<VkDescriptorType, std::uint32_t> type_counts;
		for (auto &l : set_layouts) {
			auto it = type_counts.find(l.get_type());
			if (it != type_counts.end())
				it->second += l.get_count();
			else
				type_counts[l.get_type()] = l.get_count();
		}

		std::vector<VkDescriptorPoolSize> set_sizes;
		set_sizes.reserve(type_counts.size());
		for (auto &p : type_counts) {
			VkDescriptorPoolSize size = {};
			size.type = p.first;
			size.descriptorCount = p.second;

			set_sizes.push_back(size);
		}

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = allow_free_individual_sets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
		create_info.maxSets = max_sets;
		create_info.poolSizeCount = set_sizes.size();
		create_info.pPoolSizes = set_sizes.data();

		VkDescriptorPool pool;
		vk_result res = vkCreateDescriptorPool(device, &create_info, nullptr, &pool);
		if (!res) {
			throw vk_exception(res);
		}

		this->pool = pool;
	}
	~vk_descriptor_pool() noexcept {
		destroy_pool();
	}

	vk_descriptor_pool(vk_descriptor_pool &&) = default;
	vk_descriptor_pool &operator=(vk_descriptor_pool &&) = default;
	vk_descriptor_pool(const vk_descriptor_pool &) = delete;
	vk_descriptor_pool &operator=(const vk_descriptor_pool &) = delete;

	void destroy_pool() {
		if (pool) {
			vkDestroyDescriptorPool(device.get(), *this, nullptr);
			pool = none;
		}
	}

	vk_descriptor_set allocate_descriptor_set(const std::vector<VkDescriptorSetLayout> &set_layouts) const {
		std::vector<VkDescriptorSetLayout> set_layout_descriptors;
		set_layout_descriptors.reserve(set_layouts.size());
		for (auto &s : set_layouts)
			set_layout_descriptors.push_back(s);

		VkDescriptorSetAllocateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		create_info.pNext = nullptr;
		create_info.descriptorPool = *this;
		create_info.descriptorSetCount = set_layout_descriptors.size();
		create_info.pSetLayouts = set_layout_descriptors.data();

		VkDescriptorSet set;
		vk_result res = vkAllocateDescriptorSets(device.get(), &create_info, &set);
		if (!res) {
			throw vk_exception(res);
		}

		return vk_descriptor_set(device, set, *this, allows_freeing_individual_sets());
	}

	void reset_pool() const {
		vk_result res = vkResetDescriptorPool(device.get(), *this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get() const { return pool.get(); }
	bool allows_freeing_individual_sets() const { return allow_free_individual_sets; }
};

}
}
