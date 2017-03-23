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
private:
	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_internal(const ste_context &ctx,
										 const boost::filesystem::path &path,
										 const VkImageUsageFlags &usage,
										 bool generate_mipmaps,
										 bool srgb) {
		auto surface = surface_io::load_surface_2d(path, srgb);
		return create_image_2d<format, resource_deferred_policy>(ctx,
																 std::move(surface),
																 usage,
																 generate_mipmaps);
	}

public:
	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&input,
								const VkImageUsageFlags &usage,
								bool generate_mipmaps = true) {
		static constexpr gli::format image_gli_format = GL::vk_format_traits<format>::gli_format;

		// Convert surface to target format
		bool need_conversion = image_gli_format != input.format();
		if (need_conversion)
			input = surface_convert()(input, image_gli_format);

		// Create image from surface
		return ste_resource<GL::device_image<2>, resource_deferred_policy>(ste_resource_create_with_lambda(),
																		   ste_resource_dont_defer(),
																		   [=, &ctx, input = std::move(input)]() mutable {
			return GL::device_image<2>::create_image_2d<format>(ctx,
																std::move(input),
																usage,
																generate_mipmaps);
		});
	}

	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								const boost::filesystem::path &path,
								const VkImageUsageFlags &usage,
								bool generate_mipmaps = true) {
		return create_image_2d_internal<format>(ctx,
												path,
												usage,
												generate_mipmaps,
												false);
	}

	template <VkFormat format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_srgb(const ste_context &ctx,
									 const boost::filesystem::path &path,
									 const VkImageUsageFlags &usage,
									 bool generate_mipmaps = true) {
		return create_image_2d_internal<format>(ctx,
												path,
												usage,
												generate_mipmaps,
												true);
	}
};

}
}
