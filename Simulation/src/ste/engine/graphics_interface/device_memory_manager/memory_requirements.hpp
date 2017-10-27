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

/**
 *	@brief	Describes the memory allocation requirements of a resource
 */
struct memory_requirements {
	byte_t			bytes;
	std::size_t		alignment;
	std::uint32_t	type_bits;

	memory_dedicated_allocation_flag dedicated{ memory_dedicated_allocation_flag::suballocate };
	
	memory_requirements() = default;
	/**
	 *	@brief	Create a memory_requirements structure from a Vulkan VkMemoryRequirements
	 */
	explicit memory_requirements(VkMemoryRequirements req) {
		bytes = byte_t(req.size);
		alignment = req.alignment;
		type_bits = req.memoryTypeBits;
	}
	/**
	*	@brief	Create a memory_requirements structure from a Vulkan VkMemoryRequirements2KHR
	*	
	*	@param	dedicated	Specifies additional information about the desired allocation policy
	*/
	explicit memory_requirements(VkMemoryRequirements2KHR req,
								 optional<VkMemoryDedicatedRequirementsKHR> dedicated)
		: memory_requirements(req.memoryRequirements)
	{
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
