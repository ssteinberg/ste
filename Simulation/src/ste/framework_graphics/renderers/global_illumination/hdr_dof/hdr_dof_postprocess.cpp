
#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <human_vision_properties.hpp>

#include <hdr_compute_minmax_task.hpp>
#include <hdr_create_histogram_task.hpp>
#include <hdr_compute_histogram_sums_task.hpp>
#include <hdr_tonemap_coc_task.hpp>
#include <hdr_bloom_blurx_task.hpp>
#include <hdr_bloom_blury_task.hpp>
#include <hdr_bokeh_blur_task.hpp>

#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

namespace ste::graphics::_internal {

template <gl::format image_format>
gl::packaged_texture<gl::image_type::image_2d> hdr_create_texture(const ste_context &ctx,
																  std::uint32_t res_divider = 1) {
	auto image = resource::surface_factory::image_empty_2d<image_format>(ctx,
																		 gl::image_usage::sampled,
																		 gl::image_layout::shader_read_only_optimal,
																		 ctx.device().get_surface().extent() / res_divider);
	return gl::make_texture(std::move(image),
							ctx.device().common_samplers_collection().linear_clamp_sampler());
}

}

gl::packaged_texture<gl::image_type::image_1d> hdr_dof_postprocess::create_hdr_vision_properties_texture(const ste_context &ctx) {
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

	return gl::packaged_texture<gl::image_type::image_1d>(std::move(image),
														  &ctx.device().common_samplers_collection().linear_clamp_sampler(),
														  gl::image_layout::shader_read_only_optimal);
}

std::unique_ptr<hdr_dof_postprocess::hdr_textures> hdr_dof_postprocess::create_hdr_textures(const ste_context &ctx) {
	std::unique_ptr<hdr_textures> textures = std::make_unique<hdr_textures>(hdr_textures{});

	textures->hdr_final_image = resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx,
																										   gl::image_usage::sampled,
																										   gl::image_layout::shader_read_only_optimal,
																										   ctx.device().get_surface().extent());
	textures->hdr_final_linear = gl::make_texture<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(textures->hdr_final_image),
																			ctx.device().common_samplers_collection().linear_sampler());
	textures->hdr_final_linear_clamp = gl::make_texture<gl::image_type::image_2d>(gl::image_view<gl::image_type::image_2d>(textures->hdr_final_image),
																				  ctx.device().common_samplers_collection().linear_clamp_sampler());
	textures->hdr_vision_properties_texture = create_hdr_vision_properties_texture(ctx);
	textures->hdr_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx);
	textures->hdr_bloom_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx);
	textures->hdr_bloom_blurx_image = _internal::hdr_create_texture<gl::format::r16g16b16a16_sfloat>(ctx);
	textures->hdr_lums = _internal::hdr_create_texture<gl::format::r32_sfloat>(ctx, 4);

	return textures;
}

hdr_dof_postprocess::hdr_dof_postprocess(const ste_context &ctx)
	: ctx(ctx),
	hdr_bokeh_param_buffer(ctx, 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
	hdr_bokeh_param_buffer_prev(ctx, 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
	histogram(ctx, 128, gl::buffer_usage::storage_buffer),
	histogram_sums(ctx, 128, gl::buffer_usage::storage_buffer)
//																			hdr_compute_minmax(ctx, "hdr_compute_minmax.comp"),
//																			hdr_create_histogram(ctx, "hdr_create_histogram.comp"),
//																			hdr_compute_histogram_sums(ctx, "hdr_compute_histogram_sums.comp"),
//																			hdr_tonemap_coc(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_tonemap_coc.frag" }),
//																			hdr_bloom_blurx(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_bloom_blur_x.frag" }),
//																			hdr_bloom_blury(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_bloom_blur_y.frag" }),
//																			bokeh_blur(ctx, std::vector<std::string>{ "passthrough.vert", "bokeh_bilateral_blur.frag" }),
//																			hdr_vision_properties_sampler(Core::texture_filtering::Linear, Core::texture_filtering::Linear, 16) {
{}

std::vector<std::shared_ptr<const gpu_task>> hdr_dof_postprocess::create_sub_tasks() {
	auto compute_minmax = make_gpu_task("hdr_compute_minmax", compute_minmax_task.get(), nullptr);
	auto create_histogram = make_gpu_task("hdr_create_histogram", create_histogram_task.get(), nullptr);
	auto compute_histogram_sums = make_gpu_task("hdr_compute_histogram_sums", compute_histogram_sums_task.get(), nullptr);
	auto tonemap_coc = make_gpu_task("hdr_tonemap_coc", tonemap_coc_task.get(), &fbo_hdr);
	auto bloom_blurx = make_gpu_task("hdr_bloom_blurx", bloom_blurx_task.get(), &fbo_hdr_bloom_blurx_image);
	auto bloom_blury = make_gpu_task("hdr_bloom_blury", bloom_blury_task.get(), &fbo_hdr_final);
}

hdr_dof_postprocess::~hdr_dof_postprocess() noexcept {
}

void hdr_dof_postprocess::attach_handles() const {
	auto depth_texture = gbuffer->get_depth_target();
	if (depth_texture) {
		auto depth_target_handle = depth_texture->get_texture_handle(*Core::sampler::sampler_nearest_clamp());
		depth_target_handle.make_resident();

		hdr_compute_histogram_sums.get().set_uniform("depth_texture", depth_target_handle);
		bokeh_blur.get().set_uniform("depth_texture", depth_target_handle);
	}
}

void hdr_dof_postprocess::resize(glm::ivec2 size) {
	fbo_hdr_final[0] = (*hdr_final_image)[0];
	fbo_hdr[0] = (*hdr_image)[0];
	fbo_hdr[1] = (*hdr_bloom_image)[0];
	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];

	auto hdr_handle = hdr_image->get_texture_handle();
	auto hdr_final_handle = hdr_final_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());
	auto hdr_final_handle_linear = hdr_final_image->get_texture_handle(*Core::sampler::sampler_linear());
	auto hdr_bloom_handle = hdr_bloom_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());
	auto hdr_bloom_blurx_handle = hdr_bloom_blurx_image->get_texture_handle(*Core::sampler::sampler_linear_clamp());

	hdr_handle.make_resident();
	hdr_final_handle.make_resident();
	hdr_final_handle_linear.make_resident();
	hdr_bloom_handle.make_resident();
	hdr_bloom_blurx_handle.make_resident();

	hdr_tonemap_coc.get().set_uniform("hdr", hdr_final_handle);
	hdr_bloom_blurx.get().set_uniform("hdr", hdr_bloom_handle);
	hdr_bloom_blury.get().set_uniform("hdr", hdr_bloom_blurx_handle);
	hdr_bloom_blury.get().set_uniform("unblured_hdr", hdr_handle);
	hdr_compute_minmax.get().set_uniform("hdr", hdr_final_handle_linear);
	bokeh_blur.get().set_uniform("hdr", hdr_final_handle);

	hdr_compute_histogram_sums.get().set_uniform("hdr_lum_resolution", static_cast<std::uint32_t>(luminance_size.x * luminance_size.y));
}
