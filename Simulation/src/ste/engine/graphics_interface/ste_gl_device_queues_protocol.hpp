//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_physical_device_descriptor.hpp>

#include <algorithm>
#include <vector>

namespace StE {
namespace GL {

enum class ste_gl_queue_usage {
	main_queue,
	compute_queue,
	data_transfer_queue,
	none
};

struct ste_gl_queue_descriptor {
	GL::vk_physical_device_descriptor physical_device;

	std::uint32_t family;
	ste_gl_queue_usage usage;
	std::uint32_t timestamp_valid_bits{ 0 };
	float priority{ .0f };

	auto create_device_queue_create_info() const {
		VkDeviceQueueCreateInfo device_queue_info;
		device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_info.pNext = nullptr;
		device_queue_info.flags = 0;
		device_queue_info.pQueuePriorities = &this->priority;
		device_queue_info.queueCount = 1;
		device_queue_info.queueFamilyIndex = family;

		return device_queue_info;
	}

	static auto queue_usage(const VkQueueFlags &flags) {
		if (flags & VK_QUEUE_GRAPHICS_BIT &&
			flags & VK_QUEUE_COMPUTE_BIT &&
			flags & VK_QUEUE_TRANSFER_BIT &&
			flags & VK_QUEUE_SPARSE_BINDING_BIT)
			return ste_gl_queue_usage::main_queue;
		if (flags & VK_QUEUE_COMPUTE_BIT)
			return ste_gl_queue_usage::compute_queue;
		if (flags & VK_QUEUE_TRANSFER_BIT)
			return ste_gl_queue_usage::data_transfer_queue;

		return ste_gl_queue_usage::none;
	}
};

class ste_gl_device_queues_protocol {
public:
	static std::vector<ste_gl_queue_descriptor> queues_for_physical_device(const GL::vk_physical_device_descriptor &physical_device) {
		std::vector<ste_gl_queue_descriptor> v;

		for (unsigned i = 0; i < physical_device.queue_family_properties.size(); ++i) {
			auto &q = physical_device.queue_family_properties[i];

			if (q.queueCount == 0)
				continue;

			ste_gl_queue_descriptor desc;
			desc.physical_device = physical_device;
			desc.family = i;
			desc.usage = ste_gl_queue_descriptor::queue_usage(q.queueFlags);
			desc.timestamp_valid_bits = q.timestampValidBits;

			// Ignore queues with usage flag none
			if (desc.usage == ste_gl_queue_usage::none)
				continue;

			// Ignore queues with already existing usages
			bool add = true;
			while (true) {
				auto it = std::find_if(v.begin(), v.end(), [&](const auto &d) {
					return d.usage == desc.usage;
				});
				if (it == v.end())
					break;

				// Downgrade to compute queue
				if (desc.usage == ste_gl_queue_usage::main_queue) {
					desc.usage = ste_gl_queue_usage::compute_queue;
					continue;
				}

				add = false;
				break;
			}

			// specify priority
			desc.priority = desc.usage == ste_gl_queue_usage::main_queue ? 1.f : .0f;

			if (add)
				v.push_back(desc);
		}

		// Ensure we have one queue for each type
		auto main_idx = std::find_if(v.begin(), v.end(),
						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::main_queue; }) - v.begin();
		assert(main_idx < v.size() && "No main queue created!");
		if (main_idx == v.size())
			throw ste_engine_exception("Device doesn't expose a queue with sufficient capabilities!");

		if (std::find_if(v.begin(), v.end(),
						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::compute_queue; }) == v.end())
			v.push_back(v[main_idx]);
		if (std::find_if(v.begin(), v.end(),
						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::data_transfer_queue; }) == v.end())
			v.push_back(v[main_idx]);

		return v;
	}
};

}
}
