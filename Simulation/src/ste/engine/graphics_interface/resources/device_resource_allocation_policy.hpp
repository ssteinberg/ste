//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

struct device_resource_allocation_policy {
	virtual ~device_resource_allocation_policy() noexcept {}
	virtual VkMemoryPropertyFlags required_flags() const = 0;
	virtual VkMemoryPropertyFlags preferred_flags() const = 0;
	virtual bool private_allocation() const = 0;
};

/*
*	@brief	Resource memory allocation policy for resources that live entirely on the device. Prefer device
*			local physical memory.
*/
struct device_resource_allocation_policy_device : public device_resource_allocation_policy {
	VkMemoryPropertyFlags required_flags() const override final { return 0; }
	VkMemoryPropertyFlags preferred_flags() const override final { return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
	bool private_allocation() const override final { return false; }
};

/*
*	@brief	Resource memory allocation policy for lazily-allocated resources.
*/
struct device_resource_allocation_policy_lazy : public device_resource_allocation_policy {
	VkMemoryPropertyFlags required_flags() const override final { return VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT; }
	VkMemoryPropertyFlags preferred_flags() const override final { return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
	bool private_allocation() const override final { return false; }
};

/*
*	@brief	Resource memory allocation policy for resources that require memory mapping the underlying memory to
*			a host visible virtual address.
*			Require host-visible allocation, and prefer a host cached (faster) memory type.
*			Private allocation as the mmaped allocation can't be shared.
*/
struct device_resource_allocation_policy_mmap : public device_resource_allocation_policy {
	VkMemoryPropertyFlags required_flags() const override final { return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
	VkMemoryPropertyFlags preferred_flags() const override final { return VK_MEMORY_PROPERTY_HOST_CACHED_BIT; }
	bool private_allocation() const override final { return true; }
};

/*
*	@brief	Resource memory allocation policy for resources that require coherently memory mapping the underlying 
*			memory to a host visible virtual address.
*			Similar to device_resource_allocation_policy_mmap.
*/
struct device_resource_allocation_policy_mmap_coherent : public device_resource_allocation_policy {
	VkMemoryPropertyFlags required_flags() const override final { return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
	VkMemoryPropertyFlags preferred_flags() const override final { return 0; }
	bool private_allocation() const override final { return true; }
};

}
}
