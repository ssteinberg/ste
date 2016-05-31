
#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "GLSLProgramFactory.hpp"

#include "human_vision_properties.hpp"

#include "hdr_compute_minmax_task.hpp"
#include "hdr_create_histogram_task.hpp"
#include "hdr_compute_histogram_sums_task.hpp"
#include "hdr_tonemap_coc_task.hpp"
#include "hdr_bloom_blurx_task.hpp"
#include "hdr_bloom_blury_task.hpp"
#include "hdr_bokeh_blur_task.hpp"

#include "Sampler.hpp"

using namespace StE::Graphics;

hdr_dof_postprocess::hdr_dof_postprocess(const StEngineControl &context,
										 const deferred_gbuffer *gbuffer) : gbuffer(gbuffer), ctx(context),
																			hdr_compute_minmax(ctx, "hdr_compute_minmax.glsl"),
																			hdr_create_histogram(ctx, "hdr_create_histogram.glsl"),
																			hdr_compute_histogram_sums(ctx, "hdr_compute_histogram_sums.glsl"),
																			hdr_tonemap_coc(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_tonemap_coc.frag" }),
																			hdr_bloom_blurx(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_bloom_blur_x.frag" }),
																			hdr_bloom_blury(ctx, std::vector<std::string>{ "passthrough.vert", "hdr_bloom_blur_y.frag" }),
																			bokeh_blur(ctx, std::vector<std::string>{ "passthrough.vert", "bokeh_bilateral_blur.frag" }),
																			hdr_vision_properties_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear, 16) {
	hdr_vision_properties_sampler.set_wrap_s(Core::TextureWrapMode::ClampToEdge);

	std::int32_t big_float_i = 0x7FFFFFFF;
	hdr_bokeh_param_buffer_eraser = std::make_unique<StE::Core::PixelBufferObject<std::int32_t>>(std::vector<std::int32_t>{ big_float_i, 0 });

	gli::texture1d hdr_human_vision_properties_data(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, glm::tvec1<std::size_t>{ 4096 }, 1);
	{
		glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
		for (int i = 0; i < hdr_human_vision_properties_data.extent().x; ++i, ++d) {
			float x = static_cast<float>(i) / static_cast<float>(hdr_human_vision_properties_data.extent().x);
			float l = glm::mix(StE::Graphics::human_vision_properties::min_luminance,
							   vision_properties_max_lum,
							   x);
			*d = {	StE::Graphics::human_vision_properties::scotopic_vision(l),
					StE::Graphics::human_vision_properties::mesopic_vision(l),
					StE::Graphics::human_vision_properties::monochromaticity(l),
					StE::Graphics::human_vision_properties::visual_acuity(l) };
		}
	}
	hdr_vision_properties_texture = std::make_unique<StE::Core::Texture1D>(hdr_human_vision_properties_data, false);

	task = make_gpu_task("dof_bokeh", create_dispatchable(), nullptr, create_sub_tasks());

	setup_engine_connections();
}

void hdr_dof_postprocess::setup_engine_connections() {
	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->resize(size);
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);
}

std::shared_ptr<const gpu_task> hdr_dof_postprocess::get_task() const {
	return task;
}

hdr_bokeh_blur_task* hdr_dof_postprocess::create_dispatchable() {
	bokeh_blur_task = std::make_unique<hdr_bokeh_blur_task>(this);
	return bokeh_blur_task.get();
}

std::vector<std::shared_ptr<const gpu_task>> hdr_dof_postprocess::create_sub_tasks() {
	compute_minmax_task = std::make_unique<hdr_compute_minmax_task>(this);
	create_histogram_task = std::make_unique<hdr_create_histogram_task>(this);
	compute_histogram_sums_task = std::make_unique<hdr_compute_histogram_sums_task>(this);
	tonemap_coc_task = std::make_unique<hdr_tonemap_coc_task>(this);
	bloom_blurx_task = std::make_unique<hdr_bloom_blurx_task>(this);
	bloom_blury_task = std::make_unique<hdr_bloom_blury_task>(this);

	auto compute_minmax = make_gpu_task("hdr_compute_minmax", compute_minmax_task.get(), nullptr);
	auto create_histogram = make_gpu_task("hdr_create_histogram", create_histogram_task.get(), nullptr);
	auto compute_histogram_sums = make_gpu_task("hdr_compute_histogram_sums", compute_histogram_sums_task.get(), nullptr);
	auto tonemap_coc = make_gpu_task("hdr_tonemap_coc", tonemap_coc_task.get(), &fbo_hdr);
	auto bloom_blurx = make_gpu_task("hdr_bloom_blurx", bloom_blurx_task.get(), &fbo_hdr_bloom_blurx_image);
	auto bloom_blury = make_gpu_task("hdr_bloom_blury", bloom_blury_task.get(), &fbo_hdr_final);

	bloom_blury->add_dependency(bloom_blurx);
	bloom_blurx->add_dependency(tonemap_coc);
	tonemap_coc->add_dependency(compute_histogram_sums);
	compute_histogram_sums->add_dependency(create_histogram);
	create_histogram->add_dependency(compute_minmax);

	return { bloom_blury, bloom_blurx, tonemap_coc, compute_histogram_sums, create_histogram, compute_minmax };
}

hdr_dof_postprocess::~hdr_dof_postprocess() noexcept {
}

void hdr_dof_postprocess::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	hdr_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA16_SFLOAT_PACK16, StE::Core::Texture2D::size_type(size), 1);
	hdr_final_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGB16_SFLOAT_PACK16, StE::Core::Texture2D::size_type(size), 1);
	hdr_bloom_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA16_SFLOAT_PACK16, StE::Core::Texture2D::size_type(size), 1);

	fbo_hdr_final[0] = (*hdr_final_image)[0];
	fbo_hdr[0] = (*hdr_image)[0];
	fbo_hdr[1] = (*hdr_bloom_image)[0];

	hdr_bloom_blurx_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA16_SFLOAT_PACK16, StE::Core::Texture2D::size_type(size), 1);

	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];

	luminance_size = size / 4;

	hdr_lums = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_SFLOAT_PACK32, StE::Core::Texture2D::size_type(luminance_size), 1);

	auto hdr_handle = hdr_image->get_texture_handle();
	auto hdr_final_handle = hdr_final_image->get_texture_handle(*Core::Sampler::SamplerLinearClamp());
	auto hdr_final_handle_linear = hdr_final_image->get_texture_handle(*Core::Sampler::SamplerLinear());
	auto hdr_bloom_handle = hdr_bloom_image->get_texture_handle(*Core::Sampler::SamplerLinearClamp());
	auto hdr_bloom_blurx_handle = hdr_bloom_blurx_image->get_texture_handle(*Core::Sampler::SamplerLinearClamp());

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

	hdr_bloom_blurx.get().set_uniform("size", glm::vec2{ size });
	hdr_bloom_blury.get().set_uniform("size", glm::vec2{ size });
	bokeh_blur.get().set_uniform("size", glm::vec2{ size });

	hdr_bokeh_param_buffer << *hdr_bokeh_param_buffer_eraser;
	hdr_bokeh_param_buffer_prev << *hdr_bokeh_param_buffer_eraser;
}
