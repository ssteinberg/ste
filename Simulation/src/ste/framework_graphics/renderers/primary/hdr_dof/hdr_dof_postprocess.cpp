
#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <surface_factory.hpp>
#include <combined_image_sampler.hpp>

#include <cmd_copy_buffer.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <pipeline_barrier.hpp>

using namespace ste;
using namespace ste::graphics;

namespace ste::graphics::_internal {

template <gl::format image_format>
auto hdr_create_texture(const ste_context &ctx,
						const glm::u32vec2 &extent,
						gl::image_usage usage,
						gl::image_layout layout,
						const char *name) {
	auto image = resource::surface_factory::image_empty_2d<image_format>(ctx,
																		 usage,
																		 layout,
																		 name,
																		 extent);
	return ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, std::move(image));
}

}

void hdr_dof_postprocess::bind_fragment_resources() {
	fbo_hdr_final[0] = gl::framebuffer_attachment(*input);
	fbo_hdr[0] = gl::framebuffer_attachment(hdr_image.get());
	fbo_hdr[1] = gl::framebuffer_attachment(hdr_bloom_image.get());
	fbo_hdr_bloom_blurx_image[0] = gl::framebuffer_attachment(hdr_bloom_blurx_image.get());

	const auto lums_extent = hdr_lums.get().get_image().get_extent();
	auto& samp = ctx.get().device().common_samplers_collection().linear_clamp_sampler();
	auto& samp_no_clamp = ctx.get().device().common_samplers_collection().linear_sampler();

	tonemap_coc_task.attach_framebuffer(fbo_hdr);
	bloom_blurx_task.attach_framebuffer(fbo_hdr_bloom_blurx_image);
	bloom_blury_task.attach_framebuffer(fbo_hdr_final);

	bloom_blurx_task.set_source(gl::pipeline::combined_image_sampler(hdr_bloom_image.get(), samp));
	bloom_blury_task.set_source(gl::pipeline::combined_image_sampler(hdr_image.get(), samp),
								gl::pipeline::combined_image_sampler(hdr_bloom_blurx_image.get(), samp));

	tonemap_coc_task.bind_buffers(s->histogram_sums, s->hdr_bokeh_param_buffer);
	tonemap_coc_task.set_source(gl::pipeline::combined_image_sampler(s->hdr_vision_properties_texture.get(), samp),
								gl::pipeline::combined_image_sampler(*input, samp));

	create_histogram_task.bind_buffers(s->histogram, s->hdr_bokeh_param_buffer);
	create_histogram_task.set_source(gl::pipeline::storage_image(hdr_lums.get()), lums_extent);

	bokeh_blur_task.bind_buffers(s->hdr_bokeh_param_buffer);
	bokeh_blur_task.set_source(gl::pipeline::combined_image_sampler(*input, samp));

	compute_histogram_sums_task.bind_buffers(s->histogram_sums, s->histogram, s->hdr_bokeh_param_buffer);
	compute_histogram_sums_task.set_source(lums_extent);

	adaptation_task.bind_buffers(s->hdr_bokeh_param_buffer, s->hdr_bokeh_param_buffer_prev);

	compute_minmax_task.bind_buffers(s->hdr_bokeh_param_buffer);
	compute_minmax_task.set_source(gl::pipeline::combined_image_sampler(*input, samp_no_clamp));
	compute_minmax_task.set_destination(gl::pipeline::storage_image(hdr_lums.get()), lums_extent);
}

hdr_dof_postprocess::hdr_dof_postprocess(gl::rendering_system &rs,
										 const glm::u32vec2 &extent,
										 gl::framebuffer_layout &&fb_layout)
	: ctx(rs.get_creating_context()),
	extent(extent),
	s(rs.acquire_storage<hdr_dof_postprocess_storage>()),
	// Textures
	hdr_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																			 extent, 
																			 gl::image_usage::sampled | gl::image_usage::color_attachment,
																			 gl::image_layout::shader_read_only_optimal,
																			 "hdr_image")),
	hdr_bloom_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), 
																				   extent, 
																				   gl::image_usage::sampled | gl::image_usage::color_attachment,
																				   gl::image_layout::shader_read_only_optimal,
																				   "hdr_bloom_image")),
	hdr_bloom_blurx_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), 
																						 extent, 
																						 gl::image_usage::sampled | gl::image_usage::color_attachment,
																						 gl::image_layout::shader_read_only_optimal,
																						 "hdr_bloom_blurx_image")),
	hdr_lums(_internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), 
																   extent / 4u, 
																   gl::image_usage::storage, 
																   gl::image_layout::general,
																   "hdr_lums")),
	// Fragments
	compute_minmax_task(rs),
	create_histogram_task(rs),
	adaptation_task(rs),
	compute_histogram_sums_task(rs),
	tonemap_coc_task(rs),
	bloom_blurx_task(rs),
	bloom_blury_task(rs),
	bokeh_blur_task(rs, std::move(fb_layout)),
	// Framebuffers
	fbo_hdr_final(ctx.get(), 
				  "fbo_hdr_final", 
				  bloom_blury_task.get_framebuffer_layout(), 
				  extent),
	fbo_hdr(ctx.get(), 
			"fbo_hdr",
			tonemap_coc_task.get_framebuffer_layout(), 
			extent),
	fbo_hdr_bloom_blurx_image(ctx.get(),
							  "fbo_hdr_bloom_blurx_image", 
							  bloom_blurx_task.get_framebuffer_layout(), 
							  extent)
{
	bokeh_blur_task.set_aperture_parameters(default_aperature_diameter, default_aperature_focal_ln);
}

void hdr_dof_postprocess::resize(const glm::u32vec2 &extent) {
	hdr_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), 
																			   extent, 
																			   gl::image_usage::sampled | gl::image_usage::color_attachment,
																			   gl::image_layout::shader_read_only_optimal,
																			   "hdr_image");
	hdr_bloom_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), 
																					 extent, 
																					 gl::image_usage::sampled | gl::image_usage::color_attachment,
																					 gl::image_layout::shader_read_only_optimal,
																					 "hdr_bloom_image");
	hdr_bloom_blurx_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), 
																						   extent, 
																						   gl::image_usage::sampled | gl::image_usage::color_attachment,
																						   gl::image_layout::shader_read_only_optimal,
																						   "hdr_bloom_blurx_image");
	hdr_lums = _internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), 
																	 extent / 4u, 
																	 gl::image_usage::storage, 
																	 gl::image_layout::general,
																	 "hdr_lums");

	fbo_hdr_final = gl::framebuffer(ctx.get(),
									"fbo_hdr_final",
									bloom_blury_task.get_framebuffer_layout(), 
									extent);
	fbo_hdr = gl::framebuffer(ctx.get(),
							  "fbo_hdr",
							  tonemap_coc_task.get_framebuffer_layout(), 
							  extent);
	fbo_hdr_bloom_blurx_image = gl::framebuffer(ctx.get(),
												"fbo_hdr_bloom_blurx_image",
												bloom_blurx_task.get_framebuffer_layout(), 
												extent);

	invalidated = true;
}

void hdr_dof_postprocess::record(gl::command_recorder &recorder) {
	assert(input);

	if (invalidated) {
		bind_fragment_resources();
		invalidated = false;
	}

	adaptation_task.set_tick_time_ms(tick_time_ms);

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::transfer_write)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer_prev,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::transfer_write)))
		<< gl::cmd_copy_buffer(s->hdr_bokeh_param_buffer.get(), s->hdr_bokeh_param_buffer_prev.get())
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer,
																				   gl::access_flags::transfer_read,
																				   gl::access_flags::transfer_write)))
		<< s->hdr_bokeh_param_buffer.overwrite_cmd(0, s->parameters_initial);

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer,
																				   gl::access_flags::transfer_write,
																				   gl::access_flags::shader_write),
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer_prev,
																				   gl::access_flags::transfer_write,
																				   gl::access_flags::shader_read)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::image_layout_transform_barrier(hdr_lums->get_image(),
																							gl::image_layout::general, gl::image_layout::general,
																							gl::access_flags::shader_read, gl::access_flags::shader_write)))
		<< compute_minmax_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(s->histogram,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::transfer_write)))
		<< gl::cmd_fill_buffer(s->histogram.get(), 0u)

		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(s->histogram,
																				   gl::access_flags::transfer_write,
																				   gl::access_flags::shader_write)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_read | gl::access_flags::shader_write,
																				   gl::access_flags::shader_read | gl::access_flags::shader_write),
														 gl::image_layout_transform_barrier(hdr_lums->get_image(),
																							gl::image_layout::general, gl::image_layout::general,
																							gl::access_flags::shader_write, gl::access_flags::shader_read)))
		<< create_histogram_task;

	recorder
		<< adaptation_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(s->histogram,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(s->histogram_sums,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::shader_write)))
		<< compute_histogram_sums_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::fragment_shader,
														 gl::buffer_memory_barrier(s->histogram_sums,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read),
														 gl::buffer_memory_barrier(s->hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read)))
		<< tonemap_coc_task
		<< bloom_blurx_task
		<< bloom_blury_task
		<< bokeh_blur_task;
}
