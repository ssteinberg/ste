//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <ste_engine_exceptions.hpp>
#include <ste_gl_context_creation_parameters.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_gl_device_queue.hpp>

#include <memory>
#include <vector>

namespace StE {
namespace GL {

template <typename QueueProtocol>
class ste_presentation_device {
private:
	const ste_gl_presentation_device_creation_parameters parameters;
	vk_logical_device presentation_device;
	ste_presentation_surface presentation_surface;

	std::vector<ste_gl_device_queue> device_queues;

private:
	static auto create_vk_virtual_device(const GL::vk_physical_device_descriptor &physical_device,
										 const VkPhysicalDeviceFeatures &requested_features,
										 std::vector<const char*> device_extensions = {}) {
		// Add required extensions
		device_extensions.push_back("VK_KHR_swapchain");

		// Request queues based on supplied protocol
		std::vector<VkDeviceQueueCreateInfo> queues_create_infos;
		auto queue_descriptors = QueueProtocol::queues_for_physical_device(physical_device);
		queues_create_infos.resize(queue_descriptors.size());
		for (int i = 0; i < queue_descriptors.size(); ++i)
			queues_create_infos[i] = queue_descriptors[i].create_device_queue_create_info();

		// Create logical device
		GL::vk_logical_device device(physical_device,
									 requested_features,
									 queues_create_infos,
									 device_extensions);

		return device;
	}

public:
	ste_presentation_device(const ste_gl_presentation_device_creation_parameters &parameters,
							const ste_gl_context &gl_ctx,
							const ste_window &presentation_window)
		: parameters(parameters),
		presentation_device(create_vk_virtual_device(parameters.physical_device,
													 parameters.requested_device_features,
													 parameters.additional_device_extensions)),
		presentation_surface(parameters, &presentation_device, presentation_window, gl_ctx.instance())
	{
		// Create queues
		auto queue_descriptors = QueueProtocol::queues_for_physical_device(parameters.physical_device);
		for (auto &d : queue_descriptors)
			device_queues.push_back(ste_gl_device_queue(presentation_device, d));
	}
	~ste_presentation_device() noexcept {}

	ste_presentation_device(ste_presentation_device &&) = default;
	ste_presentation_device &operator=(ste_presentation_device &&) = default;

	auto& surface() { return presentation_surface; }
	auto& surface() const { return presentation_surface; }
	auto& device() const { return presentation_device; }
	auto& queues() const { return device_queues; }
};

}
}
