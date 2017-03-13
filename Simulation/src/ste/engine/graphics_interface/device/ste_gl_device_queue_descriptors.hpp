//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <ste_engine_exceptions.hpp>

#include <vector>
#include <algorithm>
#include <boost_flatmap.hpp>
#include <functional>

namespace StE {
namespace GL {

enum class ste_gl_queue_type {
	all,
	primary_queue,
	graphics_queue,
	compute_queue,
	data_transfer_queue,
	sparse_binding_queue,
	none
};

/**
*	@brief	Returns the queue type based on the Vulkan queue flags
*/
auto inline ste_gl_queue_type_for_flags(const VkQueueFlags &flags) {
	if (flags & VK_QUEUE_GRAPHICS_BIT &&
		flags & VK_QUEUE_COMPUTE_BIT &&
		flags & VK_QUEUE_SPARSE_BINDING_BIT)
		return ste_gl_queue_type::all;

	if (flags & VK_QUEUE_GRAPHICS_BIT &&
		flags & VK_QUEUE_COMPUTE_BIT)
		// VK_QUEUE_TRANSFER_BIT is implied by VK_QUEUE_GRAPHICS_BIT
		return ste_gl_queue_type::primary_queue;

	if (flags & VK_QUEUE_GRAPHICS_BIT)
		return ste_gl_queue_type::graphics_queue;

	if (flags & VK_QUEUE_COMPUTE_BIT)
		return ste_gl_queue_type::compute_queue;

	if (flags & VK_QUEUE_TRANSFER_BIT)
		return ste_gl_queue_type::data_transfer_queue;

	if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
		return ste_gl_queue_type::sparse_binding_queue;

	return ste_gl_queue_type::none;
}

/**
*	@brief	Decay a queue type into a higher-order queue type. E.g. a compute_queue decays into a primary_queue, 
*			which decays into an 'all' placeholder in turn.
*/
ste_gl_queue_type inline ste_gl_decay_queue_type(const ste_gl_queue_type &type) {
	switch (type) {
	case ste_gl_queue_type::graphics_queue:
	case ste_gl_queue_type::compute_queue:
		return ste_gl_queue_type::primary_queue;
	case ste_gl_queue_type::data_transfer_queue:
		return ste_gl_queue_type::graphics_queue;
	default:
		return ste_gl_queue_type::all;
	}
}

/**
*	@brief	Device queue selector
*/
struct ste_gl_queue_selector {
	//	Queue type
	ste_gl_queue_type type;
	// Queue type index
	std::uint32_t type_index{ 0 };

	ste_gl_queue_selector(const ste_gl_queue_type &type) : type(type) {}
	ste_gl_queue_selector(const ste_gl_queue_type &type,
						  std::uint32_t type_index) : type(type), type_index(type_index) {}

	ste_gl_queue_selector(ste_gl_queue_selector&&) = default;
	ste_gl_queue_selector &operator=(ste_gl_queue_selector&&) = default;
	ste_gl_queue_selector(const ste_gl_queue_selector&) = default;
	ste_gl_queue_selector &operator=(const ste_gl_queue_selector&) = default;

	bool operator==(const ste_gl_queue_selector &o) const {
		return type == o.type && type_index == o.type_index;
	}
};

/**
 *	@brief	Device queue descriptor
 */
struct ste_gl_queue_descriptor {
	std::reference_wrapper<const GL::vk_physical_device_descriptor> physical_device;

	//	Queue family
	std::uint32_t family;
	// Queue priority
	float priority{ .0f };

	//	Queue type
	ste_gl_queue_type type;
	//	Queue type index (when creating multiple queues of same type)
	std::uint32_t type_index;
	//	Queue family flags
	VkQueueFlags flags;

	//	Valid bits for queue timestamps
	std::uint32_t timestamp_valid_bits{ 0 };

	ste_gl_queue_descriptor() = delete;
};

class ste_gl_queue_descriptors {
public:
	using queues_t = std::vector<ste_gl_queue_descriptor>;
	using queue_family_index_t = std::uint32_t;

	struct vk_create_info_t {
		std::vector<float> priorities;
		std::vector<VkDeviceQueueCreateInfo> create_info;
	};

private:
	using cache_key_t = std::tuple<const ste_gl_queue_descriptors*, ste_gl_queue_type, std::uint32_t>;
	using cache_t = boost::container::flat_map<cache_key_t, std::uint32_t>;
	static thread_local cache_t cached_usage_index_map;

private:
	const queues_t descriptors;

public:
	ste_gl_queue_descriptors(const queues_t &queues) : descriptors(queues) {}

	auto& operator[](int i) const { return descriptors[i]; }
	auto& get_descriptors() const { return descriptors; }

	auto size() const { return descriptors.size(); }

	auto begin() const { return descriptors.begin(); }
	auto end() const { return descriptors.end(); }

	/**
	*	@brief	Device queue descriptor
	*	
	*	@throws ste_engine_exception	If no compatible queue can be found
	*	
	*	@param	selector	Device queue selector
	*/
	std::uint32_t queue_idx(const ste_gl_queue_selector &selector) const {
		if (selector.type == ste_gl_queue_type::none ||
			selector.type == ste_gl_queue_type::all) {
			assert("ste_gl_queue_type::none or ste_gl_queue_type::all are not acceptable queue types");
		}

		auto cache_key = cache_key_t(this, selector.type, selector.type_index);
		{
			auto it = cached_usage_index_map.find(cache_key);
			if (it != cached_usage_index_map.end())
				return it->second;
		}

		auto it = std::find_if(begin(), end(), [&](const ste_gl_queue_descriptor &desc) {
			return selector == ste_gl_queue_selector{ desc.type, desc.type_index };
		});
		if (it == end()) {
			throw ste_engine_exception("Can not find compatible queue");
		}

		std::uint32_t idx = it - begin();
		cached_usage_index_map.insert(std::make_pair(cache_key, idx));
		return idx;
	}

	/**
	*	@brief	Creates Vulkan queue create info array
	*/
	auto create_device_queue_create_info() const {
		std::unique_ptr<vk_create_info_t> info = std::make_unique<vk_create_info_t>();
		info->priorities.reserve(size());
		info->create_info.reserve(size());
		
		for (auto it = begin(); it != end(); ++it) {
			auto idx = info->priorities.size();
			info->priorities.push_back(it->priority);
			for (auto next = it + 1; next != end() && next->family == it->family; ++next, ++it)
				info->priorities.push_back(next->priority);

			VkDeviceQueueCreateInfo device_queue_info = {};
			device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_info.pNext = nullptr;
			device_queue_info.flags = 0;
			device_queue_info.queueCount = info->priorities.size() - idx;
			device_queue_info.pQueuePriorities = &info->priorities[idx];
			device_queue_info.queueFamilyIndex = it->family;

			info->create_info.push_back(device_queue_info);
		}

		return info;
	}
};

}
}
