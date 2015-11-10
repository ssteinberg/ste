
#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "human_vision_properties.h"

#include <gli/gli.hpp>

using namespace StE::Graphics;

hdr_dof_postprocess::hdr_dof_postprocess(const StEngineControl &context, const LLR::Texture2D *z_buffer) : hdr_vision_properties_sampler(LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear, 16), z_buffer(z_buffer), ctx(context) {
	hdr_vision_properties_sampler.set_wrap_s(LLR::TextureWrapMode::ClampToEdge);
	linear_sampler.set_min_filter(LLR::TextureFiltering::Linear);
	linear_sampler.set_mag_filter(LLR::TextureFiltering::Linear);
	linear_mipmaps_sampler.set_min_filter(LLR::TextureFiltering::Linear);
	linear_mipmaps_sampler.set_mag_filter(LLR::TextureFiltering::Linear);
	linear_mipmaps_sampler.set_mipmap_filter(LLR::TextureFiltering::Linear);

	request_state({ GL_DEPTH_TEST, false });
	request_state({ GL_CULL_FACE, false });

	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->resize(size);
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	hdr_compute_minmax = Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_compute_minmax.glsl" })();
	hdr_create_histogram = Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_create_histogram.glsl" })();
	hdr_compute_histogram_sums = Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_compute_histogram_sums.glsl" })();
	hdr_tonemap_coc = Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_tonemap_coc.frag" })();
	hdr_bloom_blurx = Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_x.frag" })();
	hdr_bloom_blury = Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_y.frag" })();
	bokeh_blurx = Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_x.frag" })();
	bokeh_blury = Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_y.frag" })();

	gli::texture1D hdr_human_vision_properties_data(1, gli::format::FORMAT_RGBA32_SFLOAT, glm::tvec1<std::size_t>{ 4096 });
	{
		glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
		for (unsigned i = 0; i < hdr_human_vision_properties_data.dimensions().x; ++i, ++d) {
			float f = glm::mix(StE::Graphics::human_vision_properties::min_luminance, 10.f, static_cast<float>(i) / static_cast<float>(hdr_human_vision_properties_data.dimensions().x));
			*d = { StE::Graphics::human_vision_properties::scotopic_vision(f),
				StE::Graphics::human_vision_properties::red_response(f),
				StE::Graphics::human_vision_properties::monochromaticity(f),
				StE::Graphics::human_vision_properties::visual_acuity(f) };
		}
	}
	hdr_vision_properties_texture = std::make_unique<StE::LLR::Texture1D>(hdr_human_vision_properties_data, false);
	hdr_vision_properties_texture_handle = hdr_vision_properties_texture->get_texture_handle(hdr_vision_properties_sampler);

	resize(ctx.get_backbuffer_size());
}

void hdr_dof_postprocess::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	bokeh_coc = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RG32_SFLOAT, StE::LLR::Texture2D::size_type(size), 1);

	hdr_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT, StE::LLR::Texture2D::size_type(size), 1);
	hdr_final_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT, StE::LLR::Texture2D::size_type(size), 1);
	hdr_bloom_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(size), 1);

	fbo_hdr_final[0] = (*hdr_final_image)[0];
	fbo_hdr[0] = (*hdr_image)[0];
	fbo_hdr[1] = (*hdr_bloom_image)[0];
	fbo_hdr[2] = (*bokeh_coc)[0];

	hdr_bloom_blurx_image = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(size), 1);

	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];

	luminance_size = size / 4;

	float big_float = 10000.f;
	hdr_bokeh_param_buffer_eraser = std::make_unique<StE::LLR::PixelBufferObject<std::int32_t>>(std::vector<std::int32_t>{ *reinterpret_cast<std::int32_t*>(&big_float), 0 });

	hdr_lums = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_R32_SFLOAT, StE::LLR::Texture2D::size_type(luminance_size), 1);
	bokeh_blur_image_x = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(size), 1);
	fbo_bokeh_blur_image[0] = (*bokeh_blur_image_x)[0];
}

void hdr_dof_postprocess::render() const {
	using namespace LLR;

	0_sampler_idx = linear_sampler;
	1_sampler_idx = linear_mipmaps_sampler;
	unsigned zero = 0;
	histogram.clear(gli::FORMAT_R32_UINT, &zero);
	hdr_bokeh_param_buffer_prev << hdr_bokeh_param_buffer;
	hdr_bokeh_param_buffer << *hdr_bokeh_param_buffer_eraser;

	0_tex_unit = *hdr_final_image;

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

	hdr_compute_minmax->bind();
	hdr_compute_minmax->set_uniform("time", ctx.time_per_frame().count());
	0_image_idx = (*hdr_lums)[0];
	2_storage_idx = hdr_bokeh_param_buffer;
	3_storage_idx = first_frame ? hdr_bokeh_param_buffer : hdr_bokeh_param_buffer_prev;
	glDispatchCompute(luminance_size.x / 32, luminance_size .y / 32, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	hdr_create_histogram->bind();
	0_atomic_idx = histogram;
	glDispatchCompute(luminance_size.x / 32, luminance_size .y / 32, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	hdr_compute_histogram_sums->bind();
	hdr_compute_histogram_sums->set_uniform("time", ctx.time_per_frame().count());
	hdr_compute_histogram_sums->set_uniform("hdr_lum_resolution", static_cast<int>(luminance_size.x * luminance_size.y));
	0_storage_idx = histogram_sums;
	1_storage_idx = buffer_object_cast<ShaderStorageBuffer<unsigned>>(histogram);
	2_tex_unit = *z_buffer;
	glDispatchCompute(1, 1, 1);

	ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

	fbo_hdr.bind();
	hdr_tonemap_coc->bind();
	3_sampler_idx = hdr_vision_properties_sampler;
	3_tex_unit = *hdr_vision_properties_texture;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	fbo_hdr_bloom_blurx_image.bind();
	hdr_bloom_blurx->bind();
	1_tex_unit = *hdr_bloom_image;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	fbo_hdr_final.bind();
	hdr_bloom_blury->bind();
	0_tex_unit = *hdr_image;
	1_tex_unit = *hdr_bloom_blurx_image;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	0_tex_unit = *hdr_final_image;
	1_tex_unit = *bokeh_coc;
	fbo_bokeh_blur_image.bind();
	bokeh_blurx->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	bokeh_blury->bind();
	fbo->bind();
	2_tex_unit = *bokeh_blur_image_x;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	first_frame = false;
}
