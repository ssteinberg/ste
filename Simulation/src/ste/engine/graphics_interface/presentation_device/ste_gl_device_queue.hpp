//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_queues_protocol.hpp>

#include <vk_queue.hpp>

namespace StE {
namespace GL {

class ste_gl_device_queue {
private:
	vk_queue queue;
	ste_gl_queue_descriptor descriptor;

public:
	ste_gl_device_queue(const vk_logical_device &device, ste_gl_queue_descriptor descriptor)
		: queue(device, descriptor.family, 0),
		descriptor(descriptor) {}
	~ste_gl_device_queue() noexcept {}

	ste_gl_device_queue(ste_gl_device_queue &&) = default;
	ste_gl_device_queue &operator=(ste_gl_device_queue &&) = default;
	ste_gl_device_queue(const ste_gl_device_queue &) = default;
	ste_gl_device_queue &operator=(const ste_gl_device_queue &) = default;

	auto &get_queue() const { return queue; }
	auto &queue_descriptor() const { return descriptor; }
};

}
}
