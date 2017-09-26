//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

#include <optional.hpp>

namespace ste {
namespace gl {

enum class memory_dedicated_allocation_flag {
	suballocate,
	dedicated_preferred,
	dedicated_required,
};

struct memory_requirements {
	std::size_t		size;
	std::size_t		alignment;
	std::uint32_t	type_bits;

	memory_dedicated_allocation_flag dedicated{ memory_dedicated_allocation_flag::suballocate };
	
	memory_requirements() = default;
	explicit memory_requirements(VkMemoryRequirements2KHR req,
								 optional<VkMemoryDedicatedRequirementsKHR> dedicated) {
		size = req.memoryRequirements.size;
		alignment = req.memoryRequirements.alignment;
		type_bits = req.memoryRequirements.memoryTypeBits;

		if (dedicated) {
			if (dedicated.get().prefersDedicatedAllocation)
				this->dedicated = memory_dedicated_allocation_flag::dedicated_preferred;
			if (dedicated.get().requiresDedicatedAllocation)
				this->dedicated = memory_dedicated_allocation_flag::dedicated_required;
		}
	}
};

}
}
