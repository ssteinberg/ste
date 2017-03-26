//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_family.hpp>
#include <device_resource_queue_transferable.hpp>

#include <device_image_queue_transfer.hpp>
#include <device_buffer_queue_transfer.hpp>

namespace StE {
namespace GL {

void inline queue_transfer_discard(device_resource_queue_transferable &resource,
								   const ste_queue_family &dst_family) {
	resource.queue_ownership.family.store(dst_family, std::memory_order_release);
}

template <typename selector_policy>
void queue_transfer_discard(const ste_context &ctx,
							device_resource_queue_transferable &resource,
							const ste_queue_selector<selector_policy> &queue_selector) {
	auto& dst_q = ctx.device().select_queue(queue_selector);
	auto dst_family = dst_q->queue_descriptor().family;

	queue_transfer_discard(resource, dst_family);
}

}
}
