// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <device_image.hpp>

#include <image_usage.hpp>
#include <format.hpp>

#include <surface_io.hpp>
#include <surface_mipmap_generator.hpp>
#include <surface_convert.hpp>

namespace ste {
namespace resource {

class surface_factory {
private:
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_internal(const ste_context &ctx,
										 const boost::filesystem::path &path,
										 const gl::image_usage &usage,
										 bool generate_mipmaps,
										 bool srgb) {
		auto surface = surface_io::load_surface_2d(path, srgb);
		return create_image_2d<image_format, resource_deferred_policy>(ctx,
																	   std::move(surface),
																	   usage,
																	   generate_mipmaps);
	}

public:
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&input,
								const gl::image_usage &usage,
								bool generate_mipmaps = true) {
		static constexpr gli::format image_gli_format = gl::format_traits<image_format>::gli_format;

		// Convert surface to target image_format
		bool need_conversion = image_gli_format != input.format();
		if (need_conversion)
			input = surface_convert()(input, image_gli_format);

		// Create image from surface
		return ste_resource<gl::device_image<2>, resource_deferred_policy>(ste_resource_create_with_lambda(),
																		   ste_resource_dont_defer(),
																		   [=, &ctx, input = std::move(input)]() mutable {
			return gl::device_image<2>::create_image_2d<image_format>(ctx,
																	  std::move(input),
																	  usage,
																	  generate_mipmaps);
		});
	}

	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								const boost::filesystem::path &path,
								const gl::image_usage &usage,
								bool generate_mipmaps = true) {
		return create_image_2d_internal<image_format>(ctx,
													  path,
													  usage,
													  generate_mipmaps,
													  false);
	}

	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_srgb(const ste_context &ctx,
									 const boost::filesystem::path &path,
									 const gl::image_usage &usage,
									 bool generate_mipmaps = true) {
		return create_image_2d_internal<image_format>(ctx,
													  path,
													  usage,
													  generate_mipmaps,
													  true);
	}
};

}
}
