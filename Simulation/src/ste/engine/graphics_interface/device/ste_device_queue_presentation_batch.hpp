//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_device_queue_batch.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_device_presentation_sync_semaphores.hpp>

namespace StE {
namespace GL {

class ste_device_queue_presentation_batch : public ste_device_queue_batch {
	using Base = ste_device_queue_batch;

	friend class ste_device;

private:
	using Base::Base;

	ste_presentation_surface::acquire_next_image_return_t image_to_present;
	ste_device_presentation_sync_semaphores *semaphores;

public:
	auto presentation_image_index() const { return image_to_present.image_index; }
};

}
}
