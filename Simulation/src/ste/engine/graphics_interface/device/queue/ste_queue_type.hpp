//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

enum class ste_queue_type {
	all,
	primary_queue,
	graphics_queue,
	compute_queue,
	data_transfer_sparse_queue,
	data_transfer_queue,
	none
};

/**
*	@brief	Returns the queue type based on the Vulkan queue flags
*/
auto inline ste_queue_type_for_flags(const VkQueueFlags &flags) {
	if (flags & VK_QUEUE_GRAPHICS_BIT &&
		flags & VK_QUEUE_COMPUTE_BIT &&
		flags & VK_QUEUE_SPARSE_BINDING_BIT)
		return ste_queue_type::all;

	if (flags & VK_QUEUE_GRAPHICS_BIT &&
		flags & VK_QUEUE_COMPUTE_BIT)
		// VK_QUEUE_TRANSFER_BIT is implied by VK_QUEUE_GRAPHICS_BIT
		return ste_queue_type::primary_queue;

	if (flags & VK_QUEUE_GRAPHICS_BIT)
		return ste_queue_type::graphics_queue;

	if (flags & VK_QUEUE_COMPUTE_BIT)
		return ste_queue_type::compute_queue;

	if (flags & VK_QUEUE_TRANSFER_BIT &&
		flags & VK_QUEUE_SPARSE_BINDING_BIT)
		return ste_queue_type::data_transfer_sparse_queue;

	if (flags & VK_QUEUE_TRANSFER_BIT)
		return ste_queue_type::data_transfer_queue;

	return ste_queue_type::none;
}

/**
*	@brief	Decay a queue type into a higher-order queue type. E.g. a compute_queue decays into a primary_queue,
*			which decays into an 'all' placeholder in turn.
*/
ste_queue_type inline ste_decay_queue_type(const ste_queue_type &type) {
	switch (type) {
	case ste_queue_type::graphics_queue:
	case ste_queue_type::compute_queue:
		return ste_queue_type::primary_queue;
	case ste_queue_type::data_transfer_queue:
		return ste_queue_type::data_transfer_sparse_queue;
	default:
		return ste_queue_type::all;
	}
}

}
}
