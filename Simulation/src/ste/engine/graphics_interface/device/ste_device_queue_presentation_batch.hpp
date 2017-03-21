//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_device_queue_batch.hpp>
#include <ste_device_queue_batch_multishot.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_device_presentation_sync_semaphores.hpp>

#include <utility>

namespace StE {
namespace GL {

class ste_device_queue_presentation_batch_base {
	friend class ste_device;

private:
	ste_presentation_surface::acquire_next_image_return_t image_to_present;
	ste_device_presentation_sync_semaphores *semaphores{ nullptr };

protected:
	ste_device_queue_presentation_batch_base() = default;
	ste_device_queue_presentation_batch_base(const ste_presentation_surface::acquire_next_image_return_t &image_to_present,
											 ste_device_presentation_sync_semaphores *semaphores)
		: image_to_present(image_to_present),
		semaphores(semaphores)
	{}

	void set_next_image(ste_presentation_surface::acquire_next_image_return_t image,
						ste_device_presentation_sync_semaphores *semaphores) {
		this->image_to_present = std::move(image);
		this->semaphores = semaphores;
	}

public:
	auto presentation_image_index() const { return image_to_present.image_index; }
};

template <typename UserData = void>
class ste_device_queue_presentation_batch : public ste_device_queue_batch_oneshot<UserData>, public ste_device_queue_presentation_batch_base {
	using Base = ste_device_queue_batch_oneshot<UserData>;

	friend class ste_device;

private:
	struct batch_ctor {};

public:
	template <typename... Ts>
	ste_device_queue_presentation_batch(std::uint32_t queue_index,
										typename Base::pool_t &&pool,
										batch_ctor,
										const ste_presentation_surface::acquire_next_image_return_t &image_to_present,
										ste_device_presentation_sync_semaphores *semaphores,
										Ts&&... ts)
		: Base(queue_index,
			   std::move(pool),
			   std::forward<Ts>(ts)...),
		ste_device_queue_presentation_batch_base(image_to_present, semaphores)
	{}

	ste_device_queue_presentation_batch(ste_device_queue_presentation_batch&&) = default;
	ste_device_queue_presentation_batch &operator=(ste_device_queue_presentation_batch&&) = default;

	~ste_device_queue_presentation_batch() noexcept {}
};

}
}
