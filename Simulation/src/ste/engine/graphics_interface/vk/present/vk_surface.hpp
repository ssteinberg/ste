//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <optional.hpp>

#include <vulkan/vulkan.h>
#include <vk_instance.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <ste_window.hpp>

namespace StE {
namespace GL {

class vk_surface {
private:
	optional<VkSurfaceKHR> surface;
	const vk_instance &instance;

public:
	vk_surface(const ste_window &win, const vk_instance &instance) : instance(instance) {
		VkSurfaceKHR surface;
		auto handle = win.get_window_handle();
		vk_result res = glfwCreateWindowSurface(instance, handle, nullptr, &surface);
		if (!res) {
			throw vk_exception(res);
		}

		this->surface = surface;
	}
	~vk_surface() noexcept { destroy_surface(); }

	vk_surface(vk_surface &&) = default;
	vk_surface &operator=(vk_surface &&) = default;
	vk_surface(const vk_surface &) = delete;
	vk_surface &operator=(const vk_surface &) = delete;

	void destroy_surface() {
		if (surface) {
			vkDestroySurfaceKHR(instance, surface.get(), nullptr);
			surface = none;
		}
	}

	auto& get_surface() const { return surface.get(); }

	operator VkSurfaceKHR() const { return get_surface(); }
};

}
}
