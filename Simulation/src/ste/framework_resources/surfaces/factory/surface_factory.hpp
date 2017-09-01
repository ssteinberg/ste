//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <device_image.hpp>

#include <surface_io.hpp>
#include <surface.hpp>
#include <surface_convert.hpp>

#include <fill_image.hpp>
#include <generate_mipmaps.hpp>
#include <device_image_queue_transfer.hpp>

#include <image_usage.hpp>
#include <format.hpp>
#include <format_type_traits.hpp>
#include <image_type_traits.hpp>

namespace ste {
namespace resource {

class surface_factory {
private:
	using image_allocation_policy = gl::device_resource_allocation_policy_device;

private:
	// Create an image object, fills it with data from input surface and generates mipmap.
	template <int dimensions, gl::format image_format, typename Surface>
	static auto _image_from_surface_fill_internal(const ste_context &ctx,
												  Surface &&surface,
												  const gl::image_usage &usage,
												  const gl::image_layout &layout,
												  bool generate_mipmaps = true) {
		static constexpr gl::format surface_format = Surface::surface_format;
		static constexpr gl::image_type surface_image_type = Surface::surface_image_type;

		// Convert surface to target image_format
		if (image_format != surface_format) {
			if constexpr (surface_image_type == gl::image_type::image_1d)
				surface = surface_convert::convert_1d<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_2d)
				surface = surface_convert::convert_2d<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_3d)
				surface = surface_convert::convert_3d<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_1d_array)
				surface = surface_convert::convert_1d_array<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_2d_array)
				surface = surface_convert::convert_2d_array<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_cubemap)
				surface = surface_convert::convert_cubemap<image_format>(surface);
			if constexpr (surface_image_type == gl::image_type::image_cubemap_array)
				surface = surface_convert::convert_cubemap_array<image_format>(surface);
		}

		auto layers = static_cast<std::uint32_t>(surface.layers());
		auto extent = surface.extent();

		auto m = static_cast<std::uint32_t>(surface.levels());
		auto mip_levels = generate_mipmaps ?
			static_cast<std::uint32_t>(Surface::max_levels(extent)) :
			m;

		// Create image
		gl::device_image<dimensions, image_allocation_policy>
			image(ctx, gl::image_initial_layout::unused,
				  image_format, extent, gl::image_usage::transfer_dst | gl::image_usage::transfer_src | usage,
				  mip_levels, layers);

		// Copy surface to image
		gl::fill_image(image,
					   std::move(surface),
					   layout,
					   gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::primary_queue));

		if (m < mip_levels) {
			// Generate mipmaps
			gl::generate_mipmaps(image,
								 layout,
								 layout,
								 m);
		}

		return image;
	}

	// Create a ste_resource<device_image> object with deferred creation, which loads surface from path and creates the image.
	template <gl::format image_format, class resource_deferred_policy>
	static auto _image_from_surface_2d_from_file_async_internal(const ste_context &ctx,
																const std::experimental::filesystem::path &path,
																const gl::image_usage &usage,
																const gl::image_layout &layout,
																bool generate_mipmaps,
																bool srgb) {
		// Create image from file
		return ste_resource<gl::device_image<2, image_allocation_policy>, resource_deferred_policy>(ste_resource_create_with_lambda(),
																									ctx,
																									[=, &ctx]() mutable {
			auto surface = surface_io::load_surface_2d(path, srgb);
			return _image_from_surface_fill_internal<2, image_format>(ctx,
																	  std::move(surface),
																	  usage,
																	  layout,
																	  generate_mipmaps);
		});
	}

	// Create a ste_resource<device_image> object with deferred creation.
	template <
		int dimensions,
		gl::format image_format,
		class resource_deferred_policy,
		typename Surface
	>
		static auto _image_from_surface_async_internal(const ste_context &ctx,
													   Surface &&surface,
													   const gl::image_usage &usage,
													   const gl::image_layout &layout,
													   bool generate_mipmaps = true) {
		// Create image from surface
		return ste_resource<gl::device_image<dimensions, image_allocation_policy>, resource_deferred_policy>(ste_resource_create_with_lambda(),
																											 ctx,
																											 [=, &ctx, surface = std::move(surface)]() mutable {
			return _image_from_surface_fill_internal<dimensions, image_format>(ctx,
																			   std::move(surface),
																			   usage,
																			   layout,
																			   generate_mipmaps);
		});
	}

	// Create an empty ste_resource<device_image> object with deferred creation.
	template <
		int dimensions,
		gl::format image_format,
		class resource_deferred_policy
	>
		static auto _image_empty_async_internal(const ste_context &ctx,
												const gl::image_usage &usage,
												const gl::image_layout &layout,
												const gl::image_extent_type_t<dimensions> &extent,
												std::uint32_t layers,
												std::uint32_t levels,
												bool supports_cube_views) {
		// Create image from surface
		return ste_resource<gl::device_image<dimensions, image_allocation_policy>, resource_deferred_policy>(ste_resource_create_with_lambda(),
																											 ctx,
																											 [=, &ctx]() mutable {
			// Create image
			gl::device_image<dimensions, gl::device_resource_allocation_policy_device> image(ctx, gl::image_initial_layout::unused,
																							 image_format, extent, usage,
																							 levels, layers, supports_cube_views);

			// Transfer to primary queue and desired layout
			gl::access_flags access = gl::access_flags_for_image_layout(layout);
			gl::pipeline_stage pipeline_stages = gl::all_possible_pipeline_stages_for_access_flags(access);
			auto future = gl::queue_transfer_discard(ctx,
													 image,
													 gl::ste_queue_selector<gl::ste_queue_selector_policy_strict>(gl::ste_queue_type::primary_queue),
													 pipeline_stages,
													 gl::image_layout::undefined,
													 layout, access);
			future.get();

			return image;
		});
	}

public:
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_1d(const ste_context &ctx,
									  resource::surface_1d<surface_format> &&surface,
									  const gl::image_usage &usage,
									  const gl::image_layout &layout,
									  bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<1, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_2d(const ste_context &ctx,
									  resource::surface_2d<surface_format> &&surface,
									  const gl::image_usage &usage,
									  const gl::image_layout &layout,
									  bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<2, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_3d(const ste_context &ctx,
									  resource::surface_3d<surface_format> &&surface,
									  const gl::image_usage &usage,
									  const gl::image_layout &layout,
									  bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<3, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_cube(const ste_context &ctx,
										resource::surface_cubemap<surface_format> &&surface,
										const gl::image_usage &usage,
										const gl::image_layout &layout,
										bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<2, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_1d_array(const ste_context &ctx,
											resource::surface_1d_array<surface_format> &&surface,
											const gl::image_usage &usage,
											const gl::image_layout &layout,
											bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<1, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_2d_array(const ste_context &ctx,
											resource::surface_2d_array<surface_format> &&surface,
											const gl::image_usage &usage,
											const gl::image_layout &layout,
											bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<2, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an image object,
	*			fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	surface		Surface to load from
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, gl::format surface_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_cube_array(const ste_context &ctx,
											  resource::surface_cubemap_array<surface_format> &&surface,
											  const gl::image_usage &usage,
											  const gl::image_layout &layout,
											  bool generate_mipmaps = true) {
		return _image_from_surface_async_internal<2, image_format, resource_deferred_policy>(ctx,
																							 std::move(surface),
																							 usage,
																							 layout,
																							 generate_mipmaps);
	}

	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously reads the surface from path,
	*			creates an image object, fills it and generates mipmaps.
	*
	*	@param	ctx			Context
	*	@param	path			Path to surface
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	*/
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_2d(const ste_context &ctx,
									  const std::experimental::filesystem::path &path,
									  const gl::image_usage &usage,
									  const gl::image_layout &layout,
									  bool generate_mipmaps = true) {
		return _image_from_surface_2d_from_file_async_internal<image_format, resource_deferred_policy>(ctx,
																									   path,
																									   usage,
																									   layout,
																									   generate_mipmaps,
																									   false);
	}

	/**
	 *	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously reads the surface from path,
	 *			creates an image object, fills it and generates mipmaps.
	 *			Assumes the surface is in SRGB colorspace. The resulting image is transformed to linear colorspace.
	 *
	 *	@param	ctx			Context
	 *	@param	path			Path to surface
	 *	@param	usage		Image usage flags
	 *	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	 *	@param	generate_mipmaps	If set to true, will generate mipmaps for the remainder of the mipmap tail.
	 */
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_from_surface_2d_srgb(const ste_context &ctx,
										   const std::experimental::filesystem::path &path,
										   const gl::image_usage &usage,
										   const gl::image_layout &layout,
										   bool generate_mipmaps = true) {
		return _image_from_surface_2d_from_file_async_internal<image_format, resource_deferred_policy>(ctx,
																									   path,
																									   usage,
																									   layout,
																									   generate_mipmaps,
																									   true);
	}

	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an empty image object and transforms it to the
	*			desired layout.
	*			The constructed image is created with optimal tiling and default allocation policy.
	*
	*	@param	ctx			Context
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	extent		Image extent
	*	@param	layers		Image layers
	*	@param	levels		Image mipmap levels
	*/
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_empty_1d(const ste_context &ctx,
							   const gl::image_usage &usage,
							   const gl::image_layout &layout,
							   const gl::image_extent_type_t<1> &extent,
							   std::uint32_t layers = 1,
							   std::uint32_t levels = 1) {
		return _image_empty_async_internal<1, image_format, resource_deferred_policy>(ctx,
																					  usage,
																					  layout,
																					  extent,
																					  layers,
																					  levels,
																					  false);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an empty image object and transforms it to the
	*			desired layout.
	*			The constructed image is created with optimal tiling and default allocation policy.
	*
	*	@param	ctx			Context
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	extent		Image extent
	*	@param	layers		Image layers
	*	@param	levels		Image mipmap levels
	*	@param	supports_cube_views	If set to true the created image will support cube-map views. Extent.x must equal to extent.y in that case.
	*/
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_empty_2d(const ste_context &ctx,
							   const gl::image_usage &usage,
							   const gl::image_layout &layout,
							   const gl::image_extent_type_t<2> &extent,
							   std::uint32_t layers = 1,
							   std::uint32_t levels = 1,
							   bool supports_cube_views = false) {
		return _image_empty_async_internal<2, image_format, resource_deferred_policy>(ctx,
																					  usage,
																					  layout,
																					  extent,
																					  layers,
																					  levels,
																					  supports_cube_views);
	}
	/**
	*	@brief	Constructs and returns a ste_resource<device_image> object which asynchronously creates an empty image object and transforms it to the
	*			desired layout.
	*			The constructed image is created with optimal tiling and default allocation policy.
	*
	*	@param	ctx			Context
	*	@param	usage		Image usage flags
	*	@param	layout		Image layout. This is the layout the image will be transformed to at the end of the loading process.
	*	@param	extent		Image extent
	*	@param	layers		Image layers
	*	@param	levels		Image mipmap levels
	*/
	template <gl::format image_format, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
	static auto image_empty_3d(const ste_context &ctx,
							   const gl::image_usage &usage,
							   const gl::image_layout &layout,
							   const gl::image_extent_type_t<3> &extent,
							   std::uint32_t layers = 1,
							   std::uint32_t levels = 1) {
		return _image_empty_async_internal<3, image_format, resource_deferred_policy>(ctx,
																					  usage,
																					  layout,
																					  extent,
																					  layers,
																					  levels,
																					  false);
	}
};

}
}
