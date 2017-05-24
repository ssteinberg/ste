
#include <stdafx.hpp>
#include <fxaa_postprocess.hpp>

#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

fxaa_postprocess::fxaa_postprocess(const gl::rendering_system &rs,
								   const glm::u32vec2 &extent,
								   gl::framebuffer_layout &&fb_layout)
	: Base(rs,
		   gl::device_pipeline_graphics_configurations{},
		   std::move(fb_layout),
		   "fullscreen_triangle.vert", "fxaa.frag"),
	ctx(rs.get_creating_context()),
	// Textures
	input_image(ctx,
				resource::surface_factory::image_empty_2d<input_image_format>(ctx.get(),
																			  gl::image_usage::sampled | gl::image_usage::color_attachment,
																			  gl::image_layout::shader_read_only_optimal,
																			  extent))
{
	draw_task.attach_pipeline(pipeline);

	pipeline["input_tex"] = gl::bind(gl::pipeline::combined_image_sampler(input_image.get(), 
																		  ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
}

void fxaa_postprocess::resize(const glm::u32vec2 &extent) {
	input_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																	  resource::surface_factory::image_empty_2d<input_image_format>(ctx.get(),
																																	gl::image_usage::sampled | gl::image_usage::color_attachment,
																																	gl::image_layout::shader_read_only_optimal,
																																	extent));

	pipeline["input_tex"] = gl::bind(gl::pipeline::combined_image_sampler(input_image.get(),
																		  ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
}
