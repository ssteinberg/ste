
#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <human_vision_properties.hpp>

#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

hdr_dof_postprocess::hdr_bokeh_parameters hdr_dof_postprocess::parameters_initial = { std::tuple<std::int32_t, std::int32_t, float>(0x7FFFFFFF, 0, .0f) };

namespace ste::graphics::_internal {

template <gl::format image_format>
gl::packaged_image_sampler<gl::image_type::image_2d> hdr_create_texture(const ste_context &ctx,
																		std::uint32_t res_divider = 1) {
	auto image = resource::surface_factory::image_empty_2d<image_format>(ctx,
																		 gl::image_usage::sampled,
																		 gl::image_layout::shader_read_only_optimal,
																		 ctx.device().get_surface().extent() / res_divider);
	return gl::packaged_image_sampler<gl::image_type::image_2d>(std::move(image),
																&ctx.device().common_samplers_collection().linear_clamp_sampler(),
																gl::image_layout::shader_read_only_optimal);
}

}

gl::packaged_image_sampler<gl::image_type::image_1d> hdr_dof_postprocess::create_hdr_vision_properties_texture(const ste_context &ctx) {
	static constexpr auto format = gl::format::r32g32b32a32_sfloat;
	static constexpr auto gli_format = gl::format_traits<format>::gli_format;

	gli::texture1d hdr_human_vision_properties_data(gli_format, glm::tvec1<std::size_t>{ 4096 }, 1);
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

	auto image = resource::surface_factory::image_from_surface_1d<format>(ctx,
																		  std::move(hdr_human_vision_properties_data),
																		  gl::image_usage::sampled,
																		  gl::image_layout::shader_read_only_optimal,
																		  false);
	return gl::packaged_image_sampler<gl::image_type::image_1d>(std::move(image),
																&ctx.device().common_samplers_collection().linear_clamp_sampler(),
																gl::image_layout::shader_read_only_optimal);
}

gl::framebuffer_layout hdr_dof_postprocess::create_fb_layout(gl::format f) {
	return fb_layout;
	
}
gl::framebuffer_layout hdr_dof_postprocess::create_fb_layout(gl::format f1, gl::format f2) {
	gl::framebuffer_layout fb_layout;
	fb_layout[0] = gl::ignore_store(f1,
									gl::image_layout::shader_read_only_optimal);
	fb_layout[1] = gl::ignore_store(f2,
									gl::image_layout::shader_read_only_optimal);
	return fb_layout;
}

void hdr_dof_postprocess::bind_fragment_resources() {
	tonemap_coc_task.attach_framebuffer(fbo_hdr);
	bloom_blurx_task.attach_framebuffer(fbo_hdr_bloom_blurx_image);
	bloom_blury_task.attach_framebuffer(fbo_hdr_final);

	bloom_blurx_task.set_source(hdr_bloom_image);
	bloom_blury_task.set_source(hdr_image, hdr_bloom_blurx_image);

	tonemap_coc_task.bind_buffers(histogram_sums.get(), hdr_bokeh_param_buffer.get());
}

hdr_dof_postprocess::hdr_dof_postprocess(const gl::rendering_system &rs)
	:
	ctx(rs.get_creating_context()),
	// Textures
	hdr_final_image(resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																							   gl::image_usage::sampled,
																							   gl::image_layout::shader_read_only_optimal,
																							   rs.device().get_surface().extent()).get()),
	hdr_final_linear(gl::make_combined_image_sampler<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(hdr_final_image),
																			   &rs.device().common_samplers_collection().linear_sampler(),
																			   gl::image_layout::shader_read_only_optimal)),
	hdr_final_linear_clamp(gl::make_combined_image_sampler<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(hdr_final_image),
																					 &rs.device().common_samplers_collection().linear_clamp_sampler(),
																					 gl::image_layout::shader_read_only_optimal)),
	hdr_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get())),
	hdr_bloom_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get())),
	hdr_bloom_blurx_image(_internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get())),
	hdr_lums(_internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), 4)),
	hdr_vision_properties_texture(create_hdr_vision_properties_texture(ctx.get())),
	// Buffers
	hdr_bokeh_param_buffer(ctx.get(), 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
	hdr_bokeh_param_buffer_prev(ctx.get(), 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
	histogram(ctx.get(), 128, gl::buffer_usage::storage_buffer),
	histogram_sums(ctx.get(), 128, gl::buffer_usage::storage_buffer),
	// Framebuffers
	fbo_hdr_final(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat), rs.device().get_surface().extent()),
	fbo_hdr(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat, gl::format::r16g16b16a16_sfloat), rs.device().get_surface().extent()),
	fbo_hdr_bloom_blurx_image(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat), rs.device().get_surface().extent()),
	// Fragments
	compute_minmax_task(rs),
	create_histogram_task(rs),
	compute_histogram_sums_task(rs),
	tonemap_coc_task(rs),
	bloom_blurx_task(rs),
	bloom_blury_task(rs),
	bokeh_blur_task(rs)
{
	bind_fragment_resources();
}

void hdr_dof_postprocess::record(gl::command_recorder &recorder) {

}

void hdr_dof_postprocess::resize() {
	hdr_final_image = resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																								 gl::image_usage::sampled,
																								 gl::image_layout::shader_read_only_optimal,
																								 ctx.get().device().get_surface().extent()).get();
	hdr_final_linear = gl::make_combined_image_sampler<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(hdr_final_image),
																				 &ctx.get().device().common_samplers_collection().linear_sampler(),
																				 gl::image_layout::shader_read_only_optimal);
	hdr_final_linear_clamp = gl::make_combined_image_sampler<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(hdr_final_image),
																					   &ctx.get().device().common_samplers_collection().linear_clamp_sampler(),
																					   gl::image_layout::shader_read_only_optimal);
	hdr_vision_properties_texture = create_hdr_vision_properties_texture(ctx.get());
	hdr_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get());
	hdr_bloom_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get());
	hdr_bloom_blurx_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx.get());
	hdr_lums = _internal::hdr_create_texture<gl::format::r32_sfloat>(ctx.get(), 4);

	fbo_hdr_final = gl::framebuffer(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat), ctx.get().device().get_surface().extent());
	fbo_hdr = gl::framebuffer(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat, gl::format::r16g16b16a16_sfloat), ctx.get().device().get_surface().extent());
	fbo_hdr_bloom_blurx_image = gl::framebuffer(ctx.get(), create_fb_layout(gl::format::r16g16b16a16_sfloat), ctx.get().device().get_surface().extent());

	bind_fragment_resources();

	//	fbo_hdr_final[0] = (*hdr_final_image)[0];
	//	fbo_hdr[0] = (*hdr_image)[0];
	//	fbo_hdr[1] = (*hdr_bloom_image)[0];
	//	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];
	//
	//	auto hdr_handle = hdr_image->get_texture_handle();
	//	auto hdr_final_handle = hdr_final_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());
	//	auto hdr_final_handle_linear = hdr_final_image->get_texture_handle(*Core::sampler::sampler_linear());
	//	auto hdr_bloom_handle = hdr_bloom_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());
	//	auto hdr_bloom_blurx_handle = hdr_bloom_blurx_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());
	//
	//	hdr_tonemap_coc.get().set_uniform("hdr", hdr_final_handle);
	//	hdr_bloom_blurx.get().set_uniform("hdr", hdr_bloom_handle);
	//	hdr_bloom_blury.get().set_uniform("hdr", hdr_bloom_blurx_handle);
	//	hdr_bloom_blury.get().set_uniform("unblured_hdr", hdr_handle);
	//	hdr_compute_minmax.get().set_uniform("hdr", hdr_final_handle_linear);
	//	bokeh_blur.get().set_uniform("hdr", hdr_final_handle);
	//
	//	hdr_compute_histogram_sums.get().set_uniform("hdr_lum_resolution", static_cast<std::uint32_t>(luminance_size.x * luminance_size.y));

//		hdr_tonemap_coc.get().set_uniform("hdr_vision_properties_texture", vision_handle);
//		hdr_bloom_blurx.get().set_uniform("dir", glm::vec2{ 1.f, .0f });
//		hdr_bloom_blury.get().set_uniform("dir", glm::vec2{ .0f, 1.f });
}
