//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

#include <array>

namespace StE {
namespace GL {

class vk_image_view_swizzle {
private:
	using swizzle_type = std::array<VkComponentSwizzle, 4>;

private:
	swizzle_type swizzle{ VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY };

public:
	vk_image_view_swizzle() = default;
	vk_image_view_swizzle(const swizzle_type &swizzle) : swizzle(swizzle) {}
	vk_image_view_swizzle(const VkComponentMapping &component_mapping)
		: swizzle({ component_mapping.r, component_mapping.g, component_mapping.b, component_mapping.a }) {}
	~vk_image_view_swizzle() noexcept {}

	vk_image_view_swizzle(vk_image_view_swizzle &&) = default;
	vk_image_view_swizzle(const vk_image_view_swizzle &) = default;
	vk_image_view_swizzle &operator=(vk_image_view_swizzle &&) = default;
	vk_image_view_swizzle &operator=(const vk_image_view_swizzle &) = default;

	auto &operator=(const VkComponentMapping &component_mapping) {
		auto ptr = &component_mapping.r;
		std::copy(ptr, ptr + 4, &swizzle[0]);
		return *this;
	}

	operator VkComponentMapping() const {
		return *reinterpret_cast<const VkComponentMapping*>(&swizzle[0]);
	}

	auto &r() { return swizzle[0]; }
	auto &g() { return swizzle[1]; }
	auto &b() { return swizzle[2]; }
	auto &a() { return swizzle[3]; }
	auto &r() const { return swizzle[0]; }
	auto &g() const { return swizzle[1]; }
	auto &b() const { return swizzle[2]; }
	auto &a() const { return swizzle[3]; }
};

}
}
