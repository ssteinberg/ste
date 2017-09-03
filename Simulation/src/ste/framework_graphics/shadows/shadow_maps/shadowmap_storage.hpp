// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <texture.hpp>
#include <sampler.hpp>
#include <framebuffer.hpp>

#include <light_storage.hpp>
#include <light_cascade_descriptor.hpp>

#include <surface_factory.hpp>
#include <signal.hpp>

namespace ste {
namespace graphics {

class shadowmap_storage {
private:
	alias<const ste_context> ctx;

	unsigned cube_size;
	unsigned directional_map_size;

	ste_resource<gl::texture<gl::image_type::image_cubemap_array>> shadow_depth_cube_maps;
	ste_resource<gl::texture<gl::image_type::image_2d_array>> directional_shadow_maps;
	gl::sampler shadow_depth_sampler;

	gl::framebuffer shadow_depth_cube_map_fbo;
	gl::framebuffer directional_shadow_maps_fbo;

	mutable signal<> storage_modified_signal;

private:
	static auto create_shadow_fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[gl::pipeline_depth_attachment_location] = gl::ignore_store(gl::format::d32_sfloat,
																			 gl::image_layout::depth_stencil_attachment_optimal);
		return fb_layout;
	}

public:
	shadowmap_storage(const ste_context &ctx,
					  unsigned cube_size = 1024,
					  unsigned directional_map_size = 2048)
		: ctx(ctx),
		cube_size(cube_size),
		directional_map_size(directional_map_size),

		shadow_depth_cube_maps(ctx,
							   resource::surface_factory::image_empty_cubemap<gl::format::d32_sfloat>(ctx,
																									  gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																									  gl::image_layout::shader_read_only_optimal,
																									  cube_size,
																									  static_cast<std::uint32_t>(max_active_lights_per_frame))),
		directional_shadow_maps(ctx,
								resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																								  gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																								  gl::image_layout::shader_read_only_optimal,
																								  { directional_map_size, directional_map_size },
																								  static_cast<std::uint32_t>(max_active_directional_lights_per_frame * directional_light_cascades))),
		shadow_depth_sampler(ctx.device(),
							 gl::sampler_parameter::filtering(gl::sampler_filter::linear, gl::sampler_filter::linear, gl::sampler_mipmap_mode::linear),
							 gl::sampler_parameter::address_mode(gl::sampler_address_mode::clamp_to_edge, gl::sampler_address_mode::clamp_to_edge),
							 gl::sampler_parameter::depth_compare(gl::compare_op::greater)),
		shadow_depth_cube_map_fbo(ctx, create_shadow_fb_layout(ctx), glm::uvec2{ cube_size, cube_size }),
		directional_shadow_maps_fbo(ctx, create_shadow_fb_layout(ctx), glm::uvec2{ directional_map_size, directional_map_size })
	{
		shadow_depth_cube_map_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(*shadow_depth_cube_maps);
		directional_shadow_maps_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(*directional_shadow_maps);
	}
	~shadowmap_storage() noexcept {}

	shadowmap_storage(shadowmap_storage&&) = default;

	void set_cube_count(std::uint32_t size) {
		shadow_depth_cube_maps = ste_resource<gl::texture<gl::image_type::image_cubemap_array>>(ctx.get(),
																								resource::surface_factory::image_empty_cubemap<gl::format::d32_sfloat>(ctx.get(),
																																									   gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																									   gl::image_layout::shader_read_only_optimal,
																																									   cube_size,
																																									   size));
		shadow_depth_cube_map_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(*shadow_depth_cube_maps);

		storage_modified_signal.emit();
	}
	auto get_cube_count() const { return shadow_depth_cube_maps->get_image().get_layers(); }

	void set_directional_maps_count(std::uint32_t size) {
		directional_shadow_maps = ste_resource<gl::texture<gl::image_type::image_2d_array>>(ctx.get(),
																							resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx.get(),
																																							  gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																							  gl::image_layout::shader_read_only_optimal,
																																							  { directional_map_size, directional_map_size },
																																							  size * directional_light_cascades));
		directional_shadow_maps_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(*directional_shadow_maps);;

		storage_modified_signal.emit();
	}
	auto get_directional_maps_count() const { return directional_shadow_maps->get_image().get_layers() / directional_light_cascades; }

	auto& get_cube_fbo() const { return shadow_depth_cube_map_fbo; }
	auto& get_cubemaps() const { return *shadow_depth_cube_maps; }

	auto& get_directional_maps_fbo() const { return directional_shadow_maps_fbo; }
	auto& get_directional_maps() const { return *directional_shadow_maps; }

	auto& get_shadow_sampler() const { return shadow_depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
