
#include <stdafx.hpp>
#include <ste_device.hpp>
#include <ste_device_exceptions.hpp>

using namespace ste;
using namespace ste::gl;

ste_device::queues_t ste_device::create_queues(const vk::vk_logical_device<> &device,
											   const ste_queue_descriptors &queue_descriptors,
											   ste_device_sync_primitives_pools *sync_primitives_pools) {
	queues_t q(queue_descriptors.size());

	ste_queue_family prev_family = 0xFFFFFFFF;
	std::uint32_t family_index = 0;
	for (std::size_t idx = 0; idx < queue_descriptors.size(); ++idx) {
		auto &descriptor = queue_descriptors[idx];

		// Family index is used to get the queue from Vulkan
		descriptor.family == prev_family ?
			++family_index :
			(family_index = 0);
		prev_family = descriptor.family;

		// Create the device queue
		q.emplace(q.begin() + idx,
				  device,
				  family_index,
				  descriptor,
				  static_cast<std::uint32_t>(idx),
				  &sync_primitives_pools->shared_fences());
	}

	return q;
}

void ste_device::recreate_swap_chain() {
	// Wait for all queue threads and locks them and then wait for all queues to finish processing, allowing us to recreate the swap-chain 
	// and queue safely.
	for (auto &q : device_queues) {
		q.wait_lock();
	}
	device.wait_idle();

	// Recreate swap-chain
	presentation_surface->recreate_swap_chain();

	// Unlock the queues
	for (auto &q : device_queues) {
		q.unlock();
	}

	// Signal
	queues_and_surface_recreate_signal.emit(this);
}
