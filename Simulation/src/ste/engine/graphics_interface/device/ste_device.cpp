
#include <stdafx.hpp>
#include <ste_device.hpp>

using namespace StE::GL;

vk_logical_device ste_device::create_vk_virtual_device(const vk_physical_device_descriptor &physical_device,
													   const VkPhysicalDeviceFeatures &requested_features,
													   const ste_queue_descriptors &queue_descriptors,
													   std::vector<const char*> device_extensions) {
	if (queue_descriptors.size() == 0) {
		throw ste_device_creation_exception("queue_descriptors is empty");
	}

	// Add required extensions
	device_extensions.push_back("VK_KHR_swapchain");

	// Request queues based on supplied protocol
	auto queues_create_info = queue_descriptors.create_device_queue_create_info();

	// Create logical device
	return vk_logical_device(physical_device,
							 requested_features,
							 queues_create_info->create_info,
							 device_extensions);
}

void ste_device::create_presentation_fences_storage() {
	std::vector<image_presentation_sync_t> v;
	v.reserve(get_swap_chain_images_count());
	for (std::uint32_t i = 0; i < get_swap_chain_images_count(); ++i)
		v.push_back(image_presentation_sync_t{ nullptr,
		{ sync_primitives_pools->semaphores().claim(),
					sync_primitives_pools->semaphores().claim() } });

	this->presentation_sync_primitives = std::move(v);
}

ste_device::queues_t ste_device::create_queues(const vk_logical_device &device,
											   const ste_queue_descriptors &queue_descriptors,
											   ste_device_sync_primitives_pools *sync_primitives_pools) {
	queues_t q;
	q.reserve(queue_descriptors.size());

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
		q.push_back(std::make_unique<queue_t::element_type>(device,
															family_index,
															descriptor,
															idx,
															&sync_primitives_pools->shared_fences()));
	}

	return q;
}

void ste_device::recreate_swap_chain() {
	// Wait for all queue threads and then wait for all queues to finish processing, allowing us to recreate the swap-chain 
	// and queue safely.
	wait_idle();
	
	this->presentation_sync_primitives.clear();
	// Recreate swap-chain
	presentation_surface->recreate_swap_chain();
	create_presentation_fences_storage();

	// Signal
	queues_and_surface_recreate_signal.emit(this);
}
