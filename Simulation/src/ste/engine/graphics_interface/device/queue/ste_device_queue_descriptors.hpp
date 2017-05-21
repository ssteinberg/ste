//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>
#include <ste_queue_family.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <lib/vector.hpp>
#include <algorithm>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Device queue descriptor
 */
struct ste_queue_descriptor {
	std::reference_wrapper<const vk::vk_physical_device_descriptor> physical_device;

	//	Queue family
	ste_queue_family family;
	// Queue priority
	float priority{ .0f };

	//	Queue type
	ste_queue_type type;
	//	Queue type index (when creating multiple queues of same type)
	std::uint32_t type_index;
	//	Queue family flags
	VkQueueFlags flags;

	//	Valid bits for queue timestamps
	std::uint32_t timestamp_valid_bits{ 0 };

	ste_queue_descriptor() = delete;

	ste_queue_descriptor(ste_queue_descriptor&&) = default;
	ste_queue_descriptor(const ste_queue_descriptor&) = default;
	ste_queue_descriptor &operator=(ste_queue_descriptor&&) = default;
	ste_queue_descriptor &operator=(const ste_queue_descriptor&) = default;
};

class ste_queue_descriptors {
public:
	using queues_t = lib::vector<ste_queue_descriptor>;
	using queue_family_index_t = std::uint32_t;

	struct vk_create_info_t {
		lib::vector<float> priorities;
		lib::vector<VkDeviceQueueCreateInfo> create_info;
	};

private:
	const queues_t descriptors;

private:
	decltype(auto) set_descriptors(queues_t &&queues) {
		//! ste_device expects queues to be grouped by family
		std::sort(queues.begin(), queues.end(), [](const auto &o1, const auto &o2) {
			return o1.family < o2.family;
		});

		return std::move(queues);
	}

public:
	ste_queue_descriptors(queues_t &&queues) : descriptors(set_descriptors(std::move(queues))) {}

	auto& operator[](std::size_t i) const { return descriptors[i]; }
	auto& get_descriptors() const { return descriptors; }

	auto size() const { return descriptors.size(); }

	auto begin() const { return descriptors.begin(); }
	auto end() const { return descriptors.end(); }

	/**
	*	@brief	Creates Vulkan queue create info array
	*/
	auto create_device_queue_create_info() const {
		lib::unique_ptr<vk_create_info_t> info = lib::allocate_unique<vk_create_info_t>();
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
			device_queue_info.queueCount = static_cast<std::uint32_t>(info->priorities.size() - idx);
			device_queue_info.pQueuePriorities = &info->priorities[idx];
			device_queue_info.queueFamilyIndex = static_cast<std::uint32_t>(it->family);

			info->create_info.push_back(device_queue_info);
		}

		return info;
	}
};

}
}
