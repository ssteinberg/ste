//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>

#include <ste_engine_exceptions.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <vector>
#include <unordered_map>
#include <optional.hpp>

namespace StE {
namespace GL {

struct queue_create_descriptor {
	std::uint32_t count;
	float priority;
};

class ste_device_queues_protocol {
private:
	using queue_create_info_t = std::unordered_map<ste_queue_type, queue_create_descriptor>;

private:
	static auto default_queue_create_parameters(const GL::vk_physical_device_descriptor &physical_device) {
		queue_create_info_t params;
		// By default attempt to create a single high-priority primary queue, a low-priority compute and a 
		// low-priority data transfer queues.
		params[ste_queue_type::primary_queue] = { 1, 1.f };
		params[ste_queue_type::compute_queue] = { 1, .0f };
		params[ste_queue_type::data_transfer_queue] = { 1, .0f };
		if (physical_device.features.sparseBinding) {
			// If sparse binding is supported, also create a sparse bind queue
			params[ste_queue_type::sparse_binding_queue] = { 1, .0f };
		}

		return params;
	}

	static ste_queue_descriptors::queues_t
		create_queue_descriptor(const GL::vk_physical_device_descriptor &physical_device,
								const VkQueueFamilyProperties &q,
								std::uint32_t family_idx,
								const ste_queue_type &type,
								const queue_create_descriptor &info) {
		// Ignore queues with usage flag none
		if (type == ste_queue_type::none)
			return{};

		// Create descriptor
		ste_queue_descriptor desc = { std::reference_wrapper<const GL::vk_physical_device_descriptor>(physical_device) };
		desc.flags = q.queueFlags;
		desc.family = family_idx;
		desc.type = type;
		desc.timestamp_valid_bits = q.timestampValidBits;
		desc.priority = info.priority;

		// Add the desired amount of queue of this type
		ste_queue_descriptors::queues_t v;
		v.reserve(info.count);
		for (int t_idx = 0; t_idx < info.count; ++t_idx) {
			desc.type_index = t_idx;
			v.push_back(desc);
		}

		return v;
	}

	static auto find_device_queue(const GL::vk_physical_device_descriptor &physical_device,
								  const ste_queue_type &type,
								  const ste_queue_descriptors::queues_t &v,
								  ste_queue_descriptors::queues_t::const_iterator &queues_insertion_hint) {
		const VkQueueFamilyProperties *q = nullptr;
		std::uint32_t family_idx = 0;
		for (; family_idx < physical_device.queue_family_properties.size(); ++family_idx) {
			auto& queue_properties = physical_device.queue_family_properties[family_idx];
			auto queue_type = ste_queue_type_for_flags(queue_properties.queueFlags);

			if (queue_type == type) {
				// Check that we have enough unallocated queue of this type
				auto available = queue_properties.queueCount;
				auto it = v.begin();
				for (; it != v.end(); ++it) {
					if (it->family == family_idx) {
						queues_insertion_hint = it + 1;
						--available;
					}
				}

				if (available > 0) {
					// Found
					q = &queue_properties;
					break;
				}
			}
		}

		return std::make_pair(family_idx, q);
	}

public:
	/**
	*	@brief	Creates the queue descriptors.
	*			Queue descriptors are used by the device to create the device queues.
	*
	*	@throws ste_engine_exception	If queue creation parameters could not be satisfied using the specified physical device
	*
	*	@param physical_device		The physical device
	*	@param queues_create_info	Queue descriptors creation parameters
	*/
	static ste_queue_descriptors
		queue_descriptors_for_physical_device(const GL::vk_physical_device_descriptor &physical_device,
											  const optional<queue_create_info_t> &queues_create_info = none) {
		// If no create parameters passed, use default
		auto default_create_info = default_queue_create_parameters(physical_device);
		const queue_create_info_t &create_info = queues_create_info ?
			queues_create_info.get() :
			default_create_info;

		ste_queue_descriptors::queues_t v;

		// Reserve and validate input
		int reserve_size = 0;
		for (auto &pair : create_info) {
			if (pair.second.count == 0) {
				throw ste_engine_exception("queue_create_descriptor: Queue count can not be zero");
			}
			if (pair.first == ste_queue_type::none ||
				pair.first == ste_queue_type::all) {
				assert("ste_queue_type::none or ste_queue_type::all are not acceptable queue types");
			}
			reserve_size += pair.second.count;
		}
		v.reserve(reserve_size);

		for (auto info_it = create_info.begin(); info_it != create_info.end(); ++info_it) {
			auto type = info_it->first;

			// Where to insert queue
			ste_queue_descriptors::queues_t::const_iterator queues_insertion_it = v.end();

			auto device_queue = find_device_queue(physical_device, type, v, queues_insertion_it);
			auto prev_type = type;
			while (device_queue.second == nullptr) {
				// Couldn't find queue
				// Decay the type (e.g. compute -> main) and try again
				auto decayed_type = ste_decay_queue_type(prev_type);
				if (decayed_type == prev_type) {
					// Can't be further decayed, no queue found...
					throw ste_engine_exception("Queue create parameters could not be satisfied");
				}
				device_queue = find_device_queue(physical_device, decayed_type, v, queues_insertion_it);
				prev_type = decayed_type;
			}

			auto family_idx = device_queue.first;
			auto& q = *device_queue.second;

			// Construct queue descriptor(s) and add to array
			auto queues_to_insert = create_queue_descriptor(physical_device,
															q, family_idx, type, info_it->second);
			v.insert(queues_insertion_it, queues_to_insert.begin(), queues_to_insert.end());
		}

		return ste_queue_descriptors(std::move(v));
	}
};

}
}
