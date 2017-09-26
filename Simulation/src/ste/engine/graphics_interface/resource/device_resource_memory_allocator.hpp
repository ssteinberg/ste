//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <type_traits>

#include <device_resource_allocation_policy.hpp>
#include <ste_gl_device_memory_allocator.hpp>
#include <vk_resource.hpp>

namespace ste {
namespace gl {

template <class allocation_policy>
struct device_resource_memory_allocator {
	static_assert(std::is_base_of<device_resource_allocation_policy, allocation_policy>::value &&
				  !std::is_same<device_resource_allocation_policy, allocation_policy>::value,
				  "allocation_policy must be derived from device_resource_allocation_policy");

	auto operator()(const ste_gl_device_memory_allocator &allocator,
					vk::vk_resource<> &resource) const {
		auto memory_requirements = resource.get_memory_requirements();
		if (allocation_policy().requires_dedicated_allocation()) {
			// If allocation policy demands a dedicated allocation, override memory requirements settings
			memory_requirements.dedicated = memory_dedicated_allocation_flag::dedicated_required;
		}

		auto allocation = allocator.allocate_device_memory_for_resource(memory_requirements,
																		allocation_policy().required_flags(),
																		allocation_policy().preferred_flags());
		assert(allocation);

		resource.bind_memory(allocation);

		return allocation;
	}

	auto operator()(const ste_gl_device_memory_allocator &allocator,
					std::uint64_t size,
					memory_requirements memory_requirements) const {
		if (allocation_policy().requires_dedicated_allocation()) {
			// If allocation policy demands a dedicated allocation, override memory requirements settings
			memory_requirements.dedicated = memory_dedicated_allocation_flag::dedicated_required;
		}


		auto allocation = allocator.allocate_device_memory(size,
														   memory_requirements,
														   allocation_policy().required_flags(),
														   allocation_policy().preferred_flags());
		assert(allocation);

		return allocation;
	}
};

}
}
