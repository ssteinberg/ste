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

template <int dimensions, class allocation_policy = device_resource_allocation_policy_device>
class device_image : public device_image_base,
	public device_resource<vk::vk_image, allocation_policy>
{
	using Base = device_resource<vk::vk_image, allocation_policy>;
	using size_type = typename image_extent_type<dimensions>::type;

public:
	static glm::uvec3 size_to_extent(const size_type &size) {
		glm::uvec3 extent = { 1,1,1 };
		for (int i = 0; i < dimensions; ++i)
			extent[i] = size[i];
		return extent;
	}

public:
	device_image(const ste_context &ctx,
				 const image_initial_layout &layout,
				 const format &image_format,
				 const size_type &size,
				 const image_usage &usage,
				 std::uint32_t mips = 1,
				 std::uint32_t layers = 1,
				 bool supports_cube_views = false,
				 bool optimal_tiling = true,
				 bool sparse = false)
		: Base(ctx,
			   layout,
			   static_cast<VkFormat>(image_format),
			   dimensions,
			   size_to_extent(size),
			   static_cast<VkImageUsageFlags>(usage),
			   mips,
			   layers,
			   supports_cube_views,
			   optimal_tiling,
			   sparse)
	{}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;

	format get_format() const override final {
		return static_cast<format>(get_image_handle().get_format());
	}
	const glm::u32vec3& get_size() const override final {
		return get_image_handle().get_size();
	}
	auto& get_mips() const {
		return get_image_handle().get_mips();
	}
	auto& get_layers() const {
		return get_image_handle().get_layers();
	}
	const vk::vk_image& get_image_handle() const override final { return *this; }
};

}
}
