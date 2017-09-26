//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <memory_properties_flags.hpp>

namespace ste {
namespace gl {

struct device_resource_allocation_policy {
	virtual ~device_resource_allocation_policy() noexcept {}
	virtual memory_properties_flags required_flags() const = 0;
	virtual memory_properties_flags preferred_flags() const = 0;
	virtual bool requires_dedicated_allocation() const = 0;
};

/*
*	@brief	Resource memory allocation policy for resources that live entirely on the device. Prefer device
*			local physical memory.
*/
struct device_resource_allocation_policy_device : public device_resource_allocation_policy {
	memory_properties_flags required_flags() const override final { return memory_properties_flags::none; }
	memory_properties_flags preferred_flags() const override final { return memory_properties_flags::device_local; }
	bool requires_dedicated_allocation() const override final { return false; }
};

/*
*	@brief	Resource memory allocation policy for lazily-allocated resources.
*/
struct device_resource_allocation_policy_lazy : public device_resource_allocation_policy {
	memory_properties_flags required_flags() const override final { return memory_properties_flags::lazily_allocated; }
	memory_properties_flags preferred_flags() const override final { return memory_properties_flags::device_local; }
	bool requires_dedicated_allocation() const override final { return false; }
};

/*
*	@brief	Resource memory allocation policy for resources that require memory mapping the underlying memory to
*			a host visible virtual address.
*			Require host-visible allocation, and prefer a host cached (faster) memory type.
*			Private allocation as the mmaped allocation can't be shared.
*/
struct device_resource_allocation_policy_host_visible : public device_resource_allocation_policy {
	memory_properties_flags required_flags() const override final { return memory_properties_flags::host_visible; }
	memory_properties_flags preferred_flags() const override final { return memory_properties_flags::host_cached; }
	bool requires_dedicated_allocation() const override final { return true; }
};

/*
*	@brief	Resource memory allocation policy for resources that require coherently memory mapping the underlying 
*			memory to a host visible virtual address.
*			Similar to device_resource_allocation_policy_mmap.
*/
struct device_resource_allocation_policy_host_visible_coherent : public device_resource_allocation_policy {
	memory_properties_flags required_flags() const override final { return memory_properties_flags::host_visible | memory_properties_flags::host_coherent; }
	memory_properties_flags preferred_flags() const override final { return memory_properties_flags::none; }
	bool requires_dedicated_allocation() const override final { return true; }
};

}
}
