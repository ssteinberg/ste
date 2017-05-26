//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <sampler.hpp>

#include <lib/alloc.hpp>
#include <atomic>

namespace ste {
namespace gl {

/**
 *	@brief	A collection of common sampler (lazy-loaded and thread-safe)
 */
class common_samplers {
private:
	using sampler_ptr = const sampler*;
	using atomic_ptr = std::atomic<sampler_ptr>;

private:
	const vk::vk_logical_device<> *device;

	mutable atomic_ptr nearest{ nullptr };
	mutable atomic_ptr linear{ nullptr };
	mutable atomic_ptr linear_mipmap{ nullptr };

	mutable atomic_ptr linear_anisotropic16{ nullptr };
	mutable atomic_ptr linear_mipmap_anisotropic16{ nullptr };

	mutable atomic_ptr nearest_clamp{ nullptr };
	mutable atomic_ptr linear_clamp{ nullptr };
	mutable atomic_ptr linear_clamp_mipmap{ nullptr };

private:
	template <typename... Args>
	static const auto& get(atomic_ptr &p,
						   Args&&... args) {
		// Try to get existing sampler
		auto ptr = p.load(std::memory_order_relaxed);
		if (ptr)
			return *ptr;

		// Create new sampler
		auto new_sampler = lib::default_alloc<sampler>::make(std::forward<Args>(args)...);
		
		// Try to store new sampler. 
		// If someone beat us to that, we dismiss the sampler object we created and return the one stored in the atomic variable.
		if (p.compare_exchange_strong(ptr, new_sampler))
			ptr = new_sampler;
		else
			delete new_sampler;

		return *ptr;
	}

	static void dealloc(atomic_ptr &p) {
		auto ptr = p.load(std::memory_order_acquire);
		if (ptr)
			lib::default_alloc<sampler>::destroy(ptr);
	}

public:
	common_samplers(const vk::vk_logical_device<> &device) : device(&device) {}
	~common_samplers() noexcept {
		dealloc(nearest);
		dealloc(linear);
		dealloc(linear_mipmap);

		dealloc(linear_anisotropic16);
		dealloc(linear_mipmap_anisotropic16);

		dealloc(nearest_clamp);
		dealloc(linear_clamp);
		dealloc(linear_clamp_mipmap);
	}

	const auto& nearest_sampler() const {
		return 	get(nearest,
					*device,
					sampler_parameter::filtering(sampler_filter::nearest,
												 sampler_filter::nearest));
	}
	const auto& linear_sampler() const {
		return 	get(linear,
					*device,
					sampler_parameter::filtering(sampler_filter::linear,
												 sampler_filter::linear));
	}
	const auto& linear_mipmap_sampler() const {
		return 	get(linear_mipmap,
					*device,
					sampler_parameter::filtering(sampler_filter::linear,
												 sampler_filter::linear,
												 sampler_mipmap_mode::linear));
	}

	const auto& linear_anisotropic16_sampler() const {
		return 	get(linear_anisotropic16,
					*device,
					sampler_parameter::filtering(sampler_filter::nearest,
												 sampler_filter::nearest),
					sampler_parameter::anisotropy(16));
	}
	const auto& linear_mipmap_anisotropic16_sampler() const {
		return get(linear_mipmap_anisotropic16,
				   *device,
				   sampler_parameter::filtering(sampler_filter::linear,
												sampler_filter::linear,
												sampler_mipmap_mode::linear),
				   sampler_parameter::anisotropy(16));
	}

	const auto& nearest_clamp_sampler() const {
		return get(nearest_clamp,
				   *device,
				   sampler_parameter::filtering(sampler_filter::nearest,
												sampler_filter::nearest),
				   sampler_parameter::address_mode(sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge));
	}
	const auto& linear_clamp_sampler() const {
		return get(linear_clamp,
				   *device,
				   sampler_parameter::filtering(sampler_filter::linear,
												sampler_filter::linear),
				   sampler_parameter::address_mode(sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge));
	}
	const auto& linear_clamp_mipmap_sampler() const {
		return get(linear_clamp_mipmap,
				   *device,
				   sampler_parameter::filtering(sampler_filter::linear,
												sampler_filter::linear,
												sampler_mipmap_mode::linear),
				   sampler_parameter::address_mode(sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge,
												   sampler_address_mode::clamp_to_edge));
	}
};

}
}
