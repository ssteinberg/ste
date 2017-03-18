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

namespace StE {
namespace GL {

class vk_descriptor_pool {
private:
	optional<VkDescriptorPool> pool;
	const vk_logical_device &device;
	bool allow_free_individual_sets;

public:
	vk_descriptor_pool(const vk_logical_device &device,
					   std::uint32_t max_sets,
					   const std::vector<vk_descriptor_set_layout_binding> &set_layouts,
					   bool allow_free_individual_sets = false) : device(device), allow_free_individual_sets(allow_free_individual_sets) {
		std::vector<VkDescriptorPoolSize> set_sizes;
		set_sizes.resize(set_layouts.size());
		for (std::size_t i = 0; i < set_layouts.size(); ++i)
			set_sizes[i] = *(set_layouts.begin() + i);

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
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
			vkDestroyDescriptorPool(device, *this, nullptr);
			pool = none;
		}
	}

	vk_descriptor_set allocate_descriptor_set(const std::vector<vk_descriptor_set_layout> &set_layouts) const {
		std::vector<VkDescriptorSetLayout> set_layout_descriptors;
		set_layout_descriptors.resize(set_layouts.size());
		for (std::size_t i = 0; i < set_layouts.size(); ++i)
			set_layout_descriptors[i] = *(set_layouts.begin() + i);

		VkDescriptorSetAllocateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		create_info.pNext = nullptr;
		create_info.descriptorPool = *this;
		create_info.descriptorSetCount = set_layout_descriptors.size();
		create_info.pSetLayouts = set_layout_descriptors.data();

		VkDescriptorSet set;
		vk_result res = vkAllocateDescriptorSets(device, &create_info, &set);
		if (!res) {
			throw vk_exception(res);
		}

		return vk_descriptor_set(set, device, *this);
	}

	void reset_pool() const {
		vk_result res = vkResetDescriptorPool(device, *this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_pool() const { return pool.get(); }
	bool allows_freeing_individual_sets() const { return allow_free_individual_sets; }

	operator VkDescriptorPool() const { return get_pool(); }
};

}
}
