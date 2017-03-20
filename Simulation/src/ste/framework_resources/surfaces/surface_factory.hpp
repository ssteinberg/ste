// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <device_image.hpp>

#include <surface_io.hpp>
#include <surface_mipmap_generator.hpp>
#include <surface_convert.hpp>

namespace StE {
namespace Resource {

class surface_factory {
public:
	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&input,
								const VkImageUsageFlags &usage) {
		static constexpr gli::format image_gli_format = GL::vk_format_traits<format>::gli_format;

		// Convert surface to target format
		bool need_conversion = image_gli_format != input.format();
		if (need_conversion)
			input = surface_convert()(input, image_gli_format);

		// Create image from surface
		return ste_resource<GL::device_image<2>, resource_deferred_policy>(ctx,
																		   usage,
																		   std::move(input),
																		   GL::device_image_from_surface<format>());
	}

	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								const boost::filesystem::path &path,
								const VkImageUsageFlags &usage,
								bool generate_mipmaps = true) {
		auto surface = surface_io::load_surface_2d(path, false);
		if (generate_mipmaps)
			surface = surface_mipmap_generator()(std::move(surface));
		return create_image_2d<format, resource_deferred_policy>(ctx,
																 std::move(surface), 
																 usage);
	}

	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_srgb(const ste_context &ctx,
									 const boost::filesystem::path &path,
									 const VkImageUsageFlags &usage,
									 bool generate_mipmaps = true) {
		auto surface = surface_io::load_surface_2d(path, true);
		if (generate_mipmaps)
			surface = surface_mipmap_generator()(std::move(surface));
		return create_image_2d<format, resource_deferred_policy>(ctx,
																 std::move(surface),
																 usage);
	}
};

}
}
