//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_base.hpp>

#include <format.hpp>
#include <image_usage.hpp>
#include <image_type_traits.hpp>

#include <vk_image.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

namespace ste {
namespace gl {

enum class device_image_flags : std::uint32_t {
	none = 0x0,
	support_cube_views = 0x1,
	linear_tiling = 0x2,
	sparse = 0x4,
};

constexpr auto operator|(const device_image_flags &lhs, const device_image_flags &rhs) {
	using T = std::underlying_type_t<device_image_flags>;
	return static_cast<device_image_flags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const device_image_flags &lhs, const device_image_flags &rhs) {
	using T = std::underlying_type_t<device_image_flags>;
	return static_cast<device_image_flags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const device_image_flags &lhs, const device_image_flags &rhs) {
	using T = std::underlying_type_t<device_image_flags>;
	return static_cast<device_image_flags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template <int dimensions, class allocation_policy = device_resource_allocation_policy_device>
class device_image : public device_image_base,
	public device_resource<vk::vk_image<>, allocation_policy>
{
	using Base = device_resource<vk::vk_image<>, allocation_policy>;
	using extent_type = typename image_extent_type<dimensions>::type;

public:
	static glm::uvec3 make_uvec3_extent(const extent_type &extent) {
		glm::uvec3 ret = { 1,1,1 };
		for (int i = 0; i < dimensions; ++i)
			ret[i] = extent[i];
		return ret;
	}

public:
	device_image(const ste_context &ctx,
				 const image_initial_layout &layout,
				 const format &image_format,
				 const extent_type &extent,
				 const image_usage &usage,
				 std::uint32_t mips,
				 std::uint32_t layers,
				 device_image_flags flags = device_image_flags::none)
		: Base(ctx,
			   layout,
			   static_cast<VkFormat>(image_format),
			   dimensions,
			   make_uvec3_extent(extent),
			   static_cast<VkImageUsageFlags>(usage),
			   mips,
			   layers,
			   (flags & device_image_flags::support_cube_views) != device_image_flags::none,
			   (flags & device_image_flags::linear_tiling) == device_image_flags::none,
			   (flags & device_image_flags::sparse) != device_image_flags::none)
	{}
	~device_image() noexcept {}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;

	format get_format() const override final {
		return static_cast<format>(get_image_handle().get_format());
	}
	const glm::u32vec3& get_extent() const override final {
		return get_image_handle().get_extent();
	}
	auto& get_mips() const {
		return get_image_handle().get_mips();
	}
	auto& get_layers() const {
		return get_image_handle().get_layers();
	}
	const vk::vk_image<>& get_image_handle() const override final { return *this; }
};

}
}
