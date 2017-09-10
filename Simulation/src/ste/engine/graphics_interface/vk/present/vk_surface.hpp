//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <optional.hpp>

#include <vulkan/vulkan.h>
#include <vk_instance.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_host_allocator.hpp>

#include <ste_window.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_surface {
private:
	optional<VkSurfaceKHR> surface;
	const vk_instance<host_allocator> *instance;

public:
	vk_surface(const ste_window &win, const vk_instance<host_allocator> &instance) : instance(&instance) {
		VkSurfaceKHR surface;
		auto handle = win.get_window_handle();
		const vk_result res = glfwCreateWindowSurface(instance, handle, &host_allocator::allocation_callbacks(), &surface);
		if (!res) {
			throw vk_exception(res);
		}

		this->surface = surface;
	}
	~vk_surface() noexcept { destroy_surface(); }

	vk_surface(vk_surface &&) = default;
	vk_surface &operator=(vk_surface &&o) noexcept {
		destroy_surface();

		surface = std::move(o.surface);
		instance = o.instance;

		return *this;
	}
	vk_surface(const vk_surface &) = delete;
	vk_surface &operator=(const vk_surface &) = delete;

	void destroy_surface() {
		if (surface) {
			vkDestroySurfaceKHR(*instance, surface.get(), &host_allocator::allocation_callbacks());
			surface = none;
		}
	}

	auto& get_surface() const { return surface.get(); }

	operator VkSurfaceKHR() const { return get_surface(); }
};

}

}
}
