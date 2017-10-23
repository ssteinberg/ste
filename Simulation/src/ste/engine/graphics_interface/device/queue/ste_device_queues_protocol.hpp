//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>

#include <ste_engine_exceptions.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <lib/vector.hpp>
#include <lib/unordered_map.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

struct queue_create_descriptor {
	std::uint32_t count;
	float priority;
};

class ste_device_queues_protocol {
public:
	using queue_create_info_t = lib::unordered_map<ste_queue_type, queue_create_descriptor>;

private:
	static auto default_queue_create_parameters(const vk::vk_physical_device_descriptor &physical_device) {
		queue_create_info_t params;
		// By default attempt to create a single high-priority primary queue and a low-priority compute queue
		params[ste_queue_type::primary_queue] = { 1, 1.f };
		params[ste_queue_type::compute_queue] = { 1, .0f };

		// Furthermore, create a low-priority data transfer queue
		params[ste_queue_type::data_transfer_queue] = { 1, .0f };
		if (physical_device.get_features().sparseBinding) {
			// If sparse binding is supported, make a sparse bind and data trasfer queue
			params[ste_queue_type::data_transfer_sparse_queue] = { 1, .0f };
		}

		return params;
	}

	static ste_queue_descriptors::queues_t
		create_queue_descriptor(const vk::vk_physical_device_descriptor &physical_device,
								const VkQueueFamilyProperties &q,
								std::uint32_t family_idx,
								const ste_queue_type &type,
								const queue_create_descriptor &info) {
		// Ignore queues with usage flag none
		if (type == ste_queue_type::none)
			return{};

		// Create descriptor
		ste_queue_descriptor desc = { std::reference_wrapper<const vk::vk_physical_device_descriptor>(physical_device), family_idx };
		desc.flags = q.queueFlags;
		desc.type = type;
		desc.timestamp_valid_bits = q.timestampValidBits;
		desc.priority = info.priority;

		// Add the desired amount of queue of this type
		ste_queue_descriptors::queues_t v;
		v.reserve(info.count);
		for (std::uint32_t t_idx = 0; t_idx < info.count; ++t_idx) {
			desc.type_index = t_idx;
			v.push_back(desc);
		}

		return v;
	}

	static auto find_device_queue(const vk::vk_physical_device_descriptor &physical_device,
								  const ste_queue_type &type,
								  const ste_queue_descriptors::queues_t &v,
								  ste_queue_descriptors::queues_t::const_iterator &queues_insertion_hint) {
		const VkQueueFamilyProperties *q = nullptr;
		std::uint32_t family_idx = 0;
		for (; family_idx < physical_device.get_queue_family_properties().size(); ++family_idx) {
			auto& queue_properties = physical_device.get_queue_family_properties()[family_idx];
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
		queue_descriptors_for_physical_device(const vk::vk_physical_device_descriptor &physical_device,
											  const optional<queue_create_info_t> &queues_create_info = none) {
		// If no create parameters passed, use default
#ifndef RENDER_DOC
		auto default_create_info = default_queue_create_parameters(physical_device);
		const queue_create_info_t &create_info = queues_create_info ?
			queues_create_info.get() :
			default_create_info;
#else
		// RenderDoc only supports one Vulkan queue (as of Sep 2017...)
		queue_create_info_t create_info;
		create_info[gl::ste_queue_type::all] = { 1, 1.f };
#endif

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
