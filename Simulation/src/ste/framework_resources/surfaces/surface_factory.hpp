// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <device_image.hpp>

#include <fill_image.hpp>
#include <generate_mipmaps.hpp>

#include <image_usage.hpp>
#include <format.hpp>
#include <format_type_traits.hpp>
#include <image_type_traits.hpp>
#include <format_rtti.hpp>

#include <surface_io.hpp>
#include <surface_convert.hpp>

namespace ste {
namespace resource {

class surface_factory {
private:
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_internal(const ste_context &ctx,
										 const boost::filesystem::path &path,
										 const gl::image_usage &usage,
										 const gl::image_layout &layout,
										 bool generate_mipmaps,
										 bool srgb) {
		auto surface = surface_io::load_surface_2d(path, srgb);
		return create_image_2d<image_format, resource_deferred_policy>(ctx,
																	   std::move(surface),
																	   usage,
																	   layout,
																	   generate_mipmaps);
	}

public:
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&input,
								const gl::image_usage &usage,
								const gl::image_layout &layout,
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
			using image_element_type = typename gl::format_traits<image_format>::element_type;

			auto src_format = input.format();
			std::uint32_t mip_levels = generate_mipmaps ? gli::levels(input.extent()) : input.levels();
			auto layers = input.layers();
			auto size = input.extent();
			if (src_format != image_gli_format) {
				throw surface_format_exception("Input image_format is different from specified image format");
			}

			// Create image
			gl::device_image<2, gl::device_resource_allocation_policy_device>
				image(ctx, gl::image_initial_layout::unused,
					  image_format, size, gl::image_usage::transfer_dst | gl::image_usage::transfer_src | usage,
					  mip_levels, layers);

			// Copy surface to image
			gl::fill_image(image,
						   std::move(input),
						   layout,
						   gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::primary_queue));

			auto m = input.levels();
			if (m < mip_levels) {
				// Generate mipmaps
				gl::generate_mipmaps(image,
									 layout,
									 layout,
									 m);
			}

			return image;
		});
	}

	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d(const ste_context &ctx,
								const boost::filesystem::path &path,
								const gl::image_usage &usage,
								const gl::image_layout &layout,
								bool generate_mipmaps = true) {
		return create_image_2d_internal<image_format>(ctx,
													  path,
													  usage,
													  layout,
													  generate_mipmaps,
													  false);
	}

	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto create_image_2d_srgb(const ste_context &ctx,
									 const boost::filesystem::path &path,
									 const gl::image_usage &usage,
									 const gl::image_layout &layout,
									 bool generate_mipmaps = true) {
		return create_image_2d_internal<image_format>(ctx,
													  path,
													  usage,
													  layout,
													  generate_mipmaps,
													  true);
	}
};

}
}
