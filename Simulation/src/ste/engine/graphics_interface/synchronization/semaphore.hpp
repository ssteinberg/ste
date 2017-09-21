//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <vk_semaphore.hpp>

#include <ste_resource_pool_traits.hpp>

#include <boundary.hpp>
#include <chrono>
#include <functional>
#include <type_traits>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class semaphore
	: public ste_resource_pool_resetable_trait<const vk::vk_logical_device<> &, const char*>,
	  public allow_type_decay<semaphore, vk::vk_semaphore<>> {
private:
	using R = void;

private:
	vk::vk_semaphore<> sem;
	boundary<R> b;

public:
	/**
	*	@brief	Construct a semaphore object
	*/
	semaphore(const vk::vk_logical_device<> &device,
			  const char *name)
		: sem(device,
			  name) {}

	~semaphore() noexcept {}

	semaphore(semaphore &&) = default;
	semaphore &operator=(semaphore &&) = default;

	/**
	*	@brief	Checks if the semaphore future is valid
	*/
	auto is_valid() const {
		return b.is_valid();
	}

	/**
	*	@brief	Resets the semaphore future
	*/
	void reset() override {
		b = boundary<R>();
	}

	/**
	*	@brief	Signals the semaphore
	*/
	void signal_host() {
		b.signal();
	}

	/**
	*	@brief	Signals the semaphore
	*
	*	@param	e		Exception to set the semaphore to
	*/
	void set_exception(const std::exception_ptr &e) {
		b.set_exception(e);
	}

	/**
	*	@brief	Wait for the semaphore future to be signaled
	*/
	template <typename S = R>
	void wait_host() const {
		b.wait();
	}

	auto &get() const { return sem; }
};

}
}
