
#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <human_vision_properties.hpp>
#include <surface_factory.hpp>

#include <cmd_copy_buffer.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <pipeline_barrier.hpp>

using namespace ste;
using namespace ste::graphics;

hdr_bokeh_parameters hdr_dof_postprocess::parameters_initial = { std::tuple<std::int32_t, std::int32_t, float>(0x7FFFFFFF, 0, .0f) };

namespace ste::graphics::_internal {

template <gl::format image_format>
auto hdr_create_texture(const ste_context &ctx,
						const glm::u32vec2 &extent,
						gl::image_usage usage = gl::image_usage::sampled,
						gl::image_layout layout = gl::image_layout::shader_read_only_optimal) {
	auto image = resource::surface_factory::image_empty_2d<image_format>(ctx,
																		 usage,
																		 layout,
																		 extent);
	return ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, std::move(image));
}

}

ste_resource<gl::texture<gl::image_type::image_1d, 2>> hdr_dof_postprocess::create_hdr_vision_properties_texture(const ste_context &ctx) {
	static constexpr auto format = gl::format::r32g32b32a32_sfloat;
	static constexpr auto gli_format = gl::format_traits<format>::gli_format;

	gli::texture2d hdr_human_vision_properties_data(gli_format, glm::tvec2<std::size_t>{ 4096, 1 }, 1);
	{
		glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
		for (int i = 0; i < hdr_human_vision_properties_data.extent().x; ++i, ++d) {
			float x = (static_cast<float>(i) + .5f) / static_cast<float>(hdr_human_vision_properties_data.extent().x);
			float l = glm::mix(ste::graphics::human_vision_properties::min_luminance,
							   vision_properties_max_lum,
							   x);
			*d = {
				ste::graphics::human_vision_properties::scotopic_vision(l),
				ste::graphics::human_vision_properties::mesopic_vision(l),
				ste::graphics::human_vision_properties::monochromaticity(l),
				ste::graphics::human_vision_properties::visual_acuity(l)
			};
		}
	}

	auto image = resource::surface_factory::image_from_surface_2d<format>(ctx,
																		  std::move(hdr_human_vision_properties_data),
																		  gl::image_usage::sampled,
																		  gl::image_layout::shader_read_only_optimal,
																		  false);
	return ste_resource<gl::texture<gl::image_type::image_1d, 2>>(ctx, std::move(image));
}

void hdr_dof_postprocess::bind_fragment_resources() {
	fbo_hdr_final[0] = gl::framebuffer_attachment(hdr_final_image.get());
	fbo_hdr[0] = gl::framebuffer_attachment(hdr_image.get());
	fbo_hdr[1] = gl::framebuffer_attachment(hdr_bloom_image.get());
	fbo_hdr_bloom_blurx_image[0] = gl::framebuffer_attachment(hdr_bloom_blurx_image.get());

	auto lums_extent = hdr_lums.get().get_image().get_extent();
	auto& samp = ctx.get().device().common_samplers_collection().linear_clamp_sampler();
	auto& samp_no_clamp = ctx.get().device().common_samplers_collection().linear_sampler();

	tonemap_coc_task.attach_framebuffer(fbo_hdr);
	bloom_blurx_task.attach_framebuffer(fbo_hdr_bloom_blurx_image);
	bloom_blury_task.attach_framebuffer(fbo_hdr_final);

	bloom_blurx_task.set_source(gl::pipeline::combined_image_sampler(hdr_bloom_image.get(), samp));
	bloom_blury_task.set_source(gl::pipeline::combined_image_sampler(hdr_image.get(), samp),
								gl::pipeline::combined_image_sampler(hdr_bloom_blurx_image.get(), samp));

	tonemap_coc_task.bind_buffers(histogram_sums, hdr_bokeh_param_buffer);
	tonemap_coc_task.set_source(gl::pipeline::combined_image_sampler(hdr_vision_properties_texture.get(), samp),
								gl::pipeline::combined_image_sampler(hdr_final_image.get(), samp));

	create_histogram_task.bind_buffers(histogram, hdr_bokeh_param_buffer);
	create_histogram_task.set_source(gl::pipeline::storage_image(hdr_lums.get()), lums_extent);

	bokeh_blur_task.bind_buffers(hdr_bokeh_param_buffer);
	bokeh_blur_task.set_source(gl::pipeline::combined_image_sampler(hdr_final_image.get(), samp));

	compute_histogram_sums_task.bind_buffers(histogram_sums, histogram, hdr_bokeh_param_buffer);
	compute_histogram_sums_task.set_source(lums_extent);

	compute_minmax_task.bind_buffers(hdr_bokeh_param_buffer);
	compute_minmax_task.set_source(gl::pipeline::combined_image_sampler(hdr_final_image.get(), samp_no_clamp));
	compute_minmax_task.set_destination(gl::pipeline::storage_image(hdr_lums.get()), lums_extent);
}

hdr_dof_postprocess::hdr_dof_postprocess(const gl::rendering_system &rs,
										 const glm::u32vec2 &extent,
										 gl::framebuffer_layout &&fb_layout)
	: ctx(rs.get_creating_context()),
	// Textures
	hdr_final_image(ctx,
					resource::surface_factory::image_empty_2d<input_image_format>(ctx.get(),
																				  gl::image_usage::sampled | gl::image_usage::color_attachment,
																				  gl::image_layout::shader_read_only_optimal,
																				  extent)),
	hdr_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment)),
	hdr_bloom_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment)),
	hdr_bloom_blurx_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment)),
	hdr_lums(_internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), extent / 4u, gl::image_usage::storage, gl::image_layout::general)),
	hdr_vision_properties_texture(create_hdr_vision_properties_texture(ctx.get())),
	// Buffers
	hdr_bokeh_param_buffer(ctx.get(), 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
//	hdr_bokeh_param_buffer_prev(ctx.get(), 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
	histogram(ctx.get(), 128, gl::buffer_usage::storage_buffer),
	histogram_sums(ctx.get(), 128, gl::buffer_usage::storage_buffer),
	// Fragments
	compute_minmax_task(rs),
	create_histogram_task(rs),
	compute_histogram_sums_task(rs),
	tonemap_coc_task(rs),
	bloom_blurx_task(rs),
	bloom_blury_task(rs),
	bokeh_blur_task(rs, std::move(fb_layout)),
	// Framebuffers
	fbo_hdr_final(ctx.get(), bloom_blury_task.get_framebuffer_layout(), extent),
	fbo_hdr(ctx.get(), tonemap_coc_task.get_framebuffer_layout(), extent),
	fbo_hdr_bloom_blurx_image(ctx.get(), bloom_blurx_task.get_framebuffer_layout(), extent)
{
	bind_fragment_resources();

	bokeh_blur_task.set_aperture_parameters(default_aperature_diameter, default_aperature_focal_ln);
}

void hdr_dof_postprocess::resize(const glm::u32vec2 &extent) {
	hdr_final_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx.get(),
																		  resource::surface_factory::image_empty_2d<input_image_format>(ctx.get(),
																																		gl::image_usage::sampled | gl::image_usage::color_attachment,
																																		gl::image_layout::shader_read_only_optimal,
																																		extent));

	hdr_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment);
	hdr_bloom_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment);
	hdr_bloom_blurx_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get(), extent, gl::image_usage::sampled | gl::image_usage::color_attachment);
	hdr_lums = _internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), extent / 4u, gl::image_usage::storage, gl::image_layout::general);

	fbo_hdr_final = gl::framebuffer(ctx.get(), bloom_blury_task.get_framebuffer_layout(), extent);
	fbo_hdr = gl::framebuffer(ctx.get(), tonemap_coc_task.get_framebuffer_layout(), extent);
	fbo_hdr_bloom_blurx_image = gl::framebuffer(ctx.get(), bloom_blurx_task.get_framebuffer_layout(), extent);

	bind_fragment_resources();
}

void hdr_dof_postprocess::record(gl::command_recorder &recorder) {
	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::transfer_write)))
//		<< gl::cmd_copy_buffer(hdr_bokeh_param_buffer.get(), hdr_bokeh_param_buffer_prev.get())
		<< hdr_bokeh_param_buffer.update_task({ parameters_initial }, 0)();

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(hdr_bokeh_param_buffer,
																				   gl::access_flags::transfer_write,
																				   gl::access_flags::shader_write)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(input_stage_flags,
														 gl::pipeline_stage::compute_shader,
														 gl::image_layout_transform_barrier(hdr_final_image,
																							input_image_layout,
																							gl::image_layout::shader_read_only_optimal)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::image_layout_transform_barrier(hdr_lums->get_image(),
																							gl::image_layout::general, gl::image_layout::general,
																							gl::access_flags::shader_read, gl::access_flags::shader_write)))
		<< compute_minmax_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::transfer,
														 gl::buffer_memory_barrier(histogram,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::transfer_write)))
		<< gl::cmd_fill_buffer(histogram.get(), 0u)

		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(histogram,
																				   gl::access_flags::transfer_write,
																				   gl::access_flags::shader_write)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read),
														 gl::image_layout_transform_barrier(hdr_lums->get_image(),
																							gl::image_layout::general, gl::image_layout::general,
																							gl::access_flags::shader_write, gl::access_flags::shader_read)))
		<< create_histogram_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(histogram,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read),
														 gl::buffer_memory_barrier(hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::shader_write)))
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
														 gl::pipeline_stage::compute_shader,
														 gl::buffer_memory_barrier(histogram_sums,
																				   gl::access_flags::shader_read,
																				   gl::access_flags::shader_write)))
		<< compute_histogram_sums_task;

	recorder
		<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
														 gl::pipeline_stage::fragment_shader,
														 gl::buffer_memory_barrier(histogram_sums,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read),
														 gl::buffer_memory_barrier(hdr_bokeh_param_buffer,
																				   gl::access_flags::shader_write,
																				   gl::access_flags::shader_read)))
		<< tonemap_coc_task
		<< bloom_blurx_task
		<< bloom_blury_task
		<< bokeh_blur_task;
}
