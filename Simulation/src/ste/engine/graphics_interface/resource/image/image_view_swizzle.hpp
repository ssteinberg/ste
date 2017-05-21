//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

#include <array>

namespace ste {
namespace gl {

enum class component_swizzle : std::uint32_t {
	identity = VK_COMPONENT_SWIZZLE_IDENTITY,
	zero = VK_COMPONENT_SWIZZLE_ZERO,
	one = VK_COMPONENT_SWIZZLE_ONE,
	r = VK_COMPONENT_SWIZZLE_R,
	g = VK_COMPONENT_SWIZZLE_G,
	b = VK_COMPONENT_SWIZZLE_B,
	a = VK_COMPONENT_SWIZZLE_A,
};

class image_view_swizzle {
private:
	using swizzle_type = std::array<component_swizzle, 4>;

private:
	swizzle_type swizzle{ component_swizzle::identity,
		component_swizzle::identity,
		component_swizzle::identity,
		component_swizzle::identity };

public:
	image_view_swizzle() = default;
	image_view_swizzle(const swizzle_type &swizzle) : swizzle(swizzle) {}
	~image_view_swizzle() noexcept {}

	image_view_swizzle(image_view_swizzle &&) = default;
	image_view_swizzle(const image_view_swizzle &) = default;
	image_view_swizzle &operator=(image_view_swizzle &&) = default;
	image_view_swizzle &operator=(const image_view_swizzle &) = default;

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
