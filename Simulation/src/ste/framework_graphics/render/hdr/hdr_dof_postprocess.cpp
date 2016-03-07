
#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "human_vision_properties.h"

#include <gli/gli.hpp>

using namespace StE::Graphics;

hdr_dof_postprocess::hdr_dof_postprocess(const StEngineControl &context, const LLR::Texture2D *z_buffer) : hdr_vision_properties_sampler(LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear, 16), ctx(context) {
	hdr_vision_properties_sampler.set_wrap_s(LLR::TextureWrapMode::ClampToEdge);
	linear_sampler.set_min_filter(LLR::TextureFiltering::Linear);
	linear_sampler.set_mag_filter(LLR::TextureFiltering::Linear);

	float big_float = 10000.f;
	hdr_bokeh_param_buffer_eraser = std::make_unique<StE::LLR::PixelBufferObject<std::int32_t>>(std::vector<std::int32_t>{ *reinterpret_cast<std::int32_t*>(&big_float), 0 });

	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->resize(size);
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	hdr_compute_minmax = Resource::GLSLProgramFactory::load_program_task(ctx, { "hdr_compute_minmax.glsl" })();
	hdr_create_histogram = Resource::GLSLProgramFactory::load_program_task(ctx, { "hdr_create_histogram.glsl" })();
	hdr_compute_histogram_sums = Resource::GLSLProgramFactory::load_program_task(ctx, { "hdr_compute_histogram_sums.glsl" })();
	hdr_tonemap_coc = Resource::GLSLProgramFactory::load_program_task(ctx, { "passthrough.vert","hdr_tonemap_coc.frag" })();
	hdr_bloom_blurx = Resource::GLSLProgramFactory::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_x.frag" })();
	hdr_bloom_blury = Resource::GLSLProgramFactory::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_y.frag" })();
	bokeh_blurx = Resource::GLSLProgramFactory::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_x.frag" })();
	bokeh_blury = Resource::GLSLProgramFactory::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_y.frag" })();

	this->set_z_buffer(z_buffer);

	gli::texture1d hdr_human_vision_properties_data(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, glm::tvec1<std::size_t>{ 4096 }, 1);
	{
		glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
		for (unsigned i = 0; i < hdr_human_vision_properties_data.extent().x; ++i, ++d) {
			float f = glm::mix(StE::Graphics::human_vision_properties::min_luminance, 10.f, static_cast<float>(i) / static_cast<float>(hdr_human_vision_properties_data.extent().x));
			*d = {	StE::Graphics::human_vision_properties::scotopic_vision(f),
					StE::Graphics::human_vision_properties::mesopic_vision(f),
					StE::Graphics::human_vision_properties::monochromaticity(f),
					StE::Graphics::human_vision_properties::visual_acuity(f) };
		}
	}
	hdr_vision_properties_texture = std::make_unique<StE::LLR::Texture1D>(hdr_human_vision_properties_data, false);

	auto vision_handle = hdr_vision_properties_texture->get_texture_handle(hdr_vision_properties_sampler);
	vision_handle.make_resident();

	hdr_tonemap_coc->set_uniform("hdr_vision_properties_texture", vision_handle);

	storage_buffers[0] = histogram_sums.get_resource_id();
	storage_buffers[1] = histogram.get_resource_id();
	storage_buffers[2] = hdr_bokeh_param_buffer.get_resource_id();
	storage_buffers[3] = hdr_bokeh_param_buffer_prev.get_resource_id();

	resize(ctx.get_backbuffer_size());
}

void hdr_dof_postprocess::set_z_buffer(const LLR::Texture2D *z_buffer) {
	this->z_buffer = z_buffer;

	auto z_handle = z_buffer->get_texture_handle();
	z_handle.make_resident();

	hdr_tonemap_coc->set_uniform("z_buffer", z_handle);
	hdr_compute_histogram_sums->set_uniform("z_buffer", z_handle);
}

void hdr_dof_postprocess::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	bokeh_coc = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RG32_SFLOAT_PACK32, StE::LLR::Texture2D::size_type(size), 1);

	hdr_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, StE::LLR::Texture2D::size_type(size), 1);
	hdr_final_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, StE::LLR::Texture2D::size_type(size), 1);
	hdr_bloom_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::LLR::Texture2D::size_type(size), 1);

	fbo_hdr_final[0] = (*hdr_final_image)[0];
	fbo_hdr[0] = (*hdr_image)[0];
	fbo_hdr[1] = (*hdr_bloom_image)[0];
	fbo_hdr[2] = (*bokeh_coc)[0];

	hdr_bloom_blurx_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::LLR::Texture2D::size_type(size), 1);

	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];

	luminance_size = size / 4;

	hdr_lums = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_R32_SFLOAT_PACK32, StE::LLR::Texture2D::size_type(luminance_size), 1);
	bokeh_blur_image_x = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::LLR::Texture2D::size_type(size), 1);
	fbo_bokeh_blur_image[0] = (*bokeh_blur_image_x)[0];

	auto bokeh_coc_handle = bokeh_coc->get_texture_handle();
	auto hdr_handle = hdr_image->get_texture_handle();
	auto hdr_final_handle = hdr_final_image->get_texture_handle();
	auto hdr_final_handle_linear = hdr_final_image->get_texture_handle(linear_sampler);
	auto hdr_bloom_handle = hdr_bloom_image->get_texture_handle();
	auto hdr_bloom_blurx_handle = hdr_bloom_blurx_image->get_texture_handle();
	auto hdr_lums_handle = hdr_lums->get_texture_handle();
	auto bokeh_blurx_handle = bokeh_blur_image_x->get_texture_handle();

	bokeh_coc_handle.make_resident();
	hdr_handle.make_resident();
	hdr_final_handle.make_resident();
	hdr_final_handle_linear.make_resident();
	hdr_bloom_handle.make_resident();
	hdr_bloom_blurx_handle.make_resident();
	hdr_lums_handle.make_resident();
	bokeh_blurx_handle.make_resident();

	hdr_tonemap_coc->set_uniform("hdr", hdr_final_handle);
	hdr_bloom_blurx->set_uniform("hdr", hdr_bloom_handle);
	hdr_bloom_blury->set_uniform("hdr", hdr_bloom_blurx_handle);
	hdr_bloom_blury->set_uniform("unblured_hdr", hdr_handle);
	hdr_compute_minmax->set_uniform("hdr", hdr_final_handle_linear);
	bokeh_blurx->set_uniform("hdr", hdr_final_handle);
	bokeh_blurx->set_uniform("zcoc_buffer", bokeh_coc_handle);
	bokeh_blury->set_uniform("hdr", bokeh_blurx_handle);
	bokeh_blury->set_uniform("zcoc_buffer", bokeh_coc_handle);

	hdr_compute_histogram_sums->set_uniform("hdr_lum_resolution", static_cast<std::uint32_t>(luminance_size.x * luminance_size.y));
}

void hdr_dof_postprocess::prepare() const {
	using namespace LLR;

	std::uint32_t zero = 0;
	histogram.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	hdr_bokeh_param_buffer_prev << hdr_bokeh_param_buffer;
	hdr_bokeh_param_buffer << *hdr_bokeh_param_buffer_eraser;

	hdr_compute_minmax->set_uniform("time", ctx.time_per_frame().count());
	hdr_compute_histogram_sums->set_uniform("time", ctx.time_per_frame().count());

	0_atomic_idx = histogram;
	0_image_idx = (*hdr_lums)[0];
	ctx.gl()->bind_buffers_base<0, 4>(GL_SHADER_STORAGE_BUFFER, storage_buffers);
	if (first_frame) {
		3_storage_idx = hdr_bokeh_param_buffer;
		first_frame = false;
	}

	ScreenFillingQuad.vao()->bind();
}

void hdr_dof_postprocess::render() const {
	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

	hdr_compute_minmax->bind();
	glDispatchCompute(luminance_size.x / 32, luminance_size .y / 32, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	hdr_create_histogram->bind();
	glDispatchCompute(luminance_size.x / 32, luminance_size .y / 32, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	hdr_compute_histogram_sums->bind();
	glDispatchCompute(1, 1, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

	fbo_hdr.bind();
	hdr_tonemap_coc->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	fbo_hdr_bloom_blurx_image.bind();
	hdr_bloom_blurx->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	fbo_hdr_final.bind();
	hdr_bloom_blury->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	fbo_bokeh_blur_image.bind();
	bokeh_blurx->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	bokeh_blury->bind();
	fbo->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
