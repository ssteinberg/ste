//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout_binding.hpp>
#include <vk_ext_debug_marker.hpp>

#include <vk_host_allocator.hpp>
#include <optional.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_descriptor_set_layout : public allow_type_decay<vk_descriptor_set_layout<host_allocator>, VkDescriptorSetLayout> {
private:
	optional<VkDescriptorSetLayout> layout;
	alias<const vk_logical_device<host_allocator>> device;

	const char *name;

public:
	vk_descriptor_set_layout(const vk_logical_device<host_allocator> &device,
							 const lib::vector<vk_descriptor_set_layout_binding> &bindings,
							 const char *name) 
		: device(device),
		name(name) 
	{
		lib::vector<VkDescriptorSetLayoutBinding> binding_descriptors;
		binding_descriptors.reserve(bindings.size());
		for (auto &b : bindings)
			binding_descriptors.push_back(b);

		VkDescriptorSetLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.bindingCount = static_cast<std::uint32_t>(binding_descriptors.size());
		create_info.pBindings = binding_descriptors.data();

		VkDescriptorSetLayout layout;
		const vk_result res = vkCreateDescriptorSetLayout(device, &create_info, &host_allocator::allocation_callbacks(), &layout);
		if (!res) {
			throw vk_exception(res);
		}

		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										layout,
										VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
										name);

		this->layout = layout;
	}
	~vk_descriptor_set_layout() noexcept {
		destroy_descriptor_set_layout();
	}

	vk_descriptor_set_layout(vk_descriptor_set_layout &&) = default;
	vk_descriptor_set_layout &operator=(vk_descriptor_set_layout &&o) noexcept {
		destroy_descriptor_set_layout();

		layout = std::move(o.layout);
		device = std::move(o.device);

		return *this;
	}
	vk_descriptor_set_layout(const vk_descriptor_set_layout &) = delete;
	vk_descriptor_set_layout &operator=(const vk_descriptor_set_layout &) = delete;

	void destroy_descriptor_set_layout() {
		if (layout) {
			vkDestroyDescriptorSetLayout(device.get(), *this, &host_allocator::allocation_callbacks());
			layout = none;
		}
	}

	auto& get() const { return layout.get(); }
	auto* get_name() const { return name; }
};

}

}
}
