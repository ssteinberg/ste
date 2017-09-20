//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_device_queue_batch.hpp>
#include <ste_presentation_surface.hpp>
#include <presentation_engine_sync_semaphores.hpp>

#include <utility>

namespace ste {
namespace gl {

class ste_device_queue_presentation_batch_base {
	friend class presentation_engine;

private:
	ste_presentation_surface::acquire_next_image_return_t image_to_present;
	presentation_engine_sync_semaphores *semaphores{ nullptr };

protected:
	ste_device_queue_presentation_batch_base() = default;

	ste_device_queue_presentation_batch_base(const ste_presentation_surface::acquire_next_image_return_t &image_to_present,
											 presentation_engine_sync_semaphores *semaphores)
		: image_to_present(image_to_present),
		  semaphores(semaphores) {}

	void set_next_image(ste_presentation_surface::acquire_next_image_return_t image,
						presentation_engine_sync_semaphores *semaphores) {
		this->image_to_present = std::move(image);
		this->semaphores = semaphores;
	}

public:
	auto presentation_image_index() const { return image_to_present.image_index; }
};

template <typename UserData = void>
class device_queue_presentation_batch : public ste_device_queue_batch<UserData>, public ste_device_queue_presentation_batch_base {
	using Base = ste_device_queue_batch<UserData>;

	friend class presentation_engine;
	friend class ste_device_queue;

private:
	using Base::ctor;

public:
	template <typename... Ts>
	device_queue_presentation_batch(ctor,
									std::uint32_t queue_index,
									typename Base::pool_t &&pool,
									typename Base::fence_ptr_strong_t &&fence,
									const ste_presentation_surface::acquire_next_image_return_t &image_to_present,
									presentation_engine_sync_semaphores *semaphores,
									Ts &&... ts)
		: Base(Base::ctor{}, 
			   queue_index,
			   std::move(pool),
			   std::move(fence),
			   std::forward<Ts>(ts)...),
		  ste_device_queue_presentation_batch_base(image_to_present, semaphores) {}

	device_queue_presentation_batch(device_queue_presentation_batch &&) = default;
	device_queue_presentation_batch &operator=(device_queue_presentation_batch &&) = default;

	~device_queue_presentation_batch() noexcept {}
};

}
}
