
#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "human_vision_properties.hpp"

#include "hdr_bokeh_blurx_task.hpp"

#include <gli/gli.hpp>

using namespace StE::Graphics;

hdr_dof_postprocess::hdr_dof_postprocess(const StEngineControl &context, const Core::Texture2D *z_buffer) : gpu_task(std::make_unique<hdr_bokeh_blurx_task>(this)),
																											hdr_vision_properties_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear, 16), 
																											ctx(context) {
	hdr_vision_properties_sampler.set_wrap_s(Core::TextureWrapMode::ClampToEdge);
	linear_sampler.set_min_filter(Core::TextureFiltering::Linear);
	linear_sampler.set_mag_filter(Core::TextureFiltering::Linear);

	float big_float = 10000.f;
	hdr_bokeh_param_buffer_eraser = std::make_unique<StE::Core::PixelBufferObject<std::int32_t>>(std::vector<std::int32_t>{ *reinterpret_cast<std::int32_t*>(&big_float), 0 });

	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		this->resize(size);
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	hdr_compute_minmax = context.glslprograms_pool().fetch_program_task({ "hdr_compute_minmax.glsl" })();
	hdr_create_histogram = context.glslprograms_pool().fetch_program_task({ "hdr_create_histogram.glsl" })();
	hdr_compute_histogram_sums = context.glslprograms_pool().fetch_program_task({ "hdr_compute_histogram_sums.glsl" })();
	hdr_tonemap_coc = context.glslprograms_pool().fetch_program_task({ "passthrough.vert","hdr_tonemap_coc.frag" })();
	hdr_bloom_blurx = context.glslprograms_pool().fetch_program_task({ "passthrough.vert","hdr_bloom_blur_x.frag" })();
	hdr_bloom_blury = context.glslprograms_pool().fetch_program_task({ "passthrough.vert","hdr_bloom_blur_y.frag" })();
	bokeh_blurx = context.glslprograms_pool().fetch_program_task({ "passthrough.vert","bokeh_bilateral_blur_x.frag" })();
	bokeh_blury = context.glslprograms_pool().fetch_program_task({ "passthrough.vert","bokeh_bilateral_blur_y.frag" })();

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
	hdr_vision_properties_texture = std::make_unique<StE::Core::Texture1D>(hdr_human_vision_properties_data, false);

	auto vision_handle = hdr_vision_properties_texture->get_texture_handle(hdr_vision_properties_sampler);
	vision_handle.make_resident();

	hdr_tonemap_coc->set_uniform("hdr_vision_properties_texture", vision_handle);

	resize(ctx.get_backbuffer_size());
}

hdr_dof_postprocess::~hdr_dof_postprocess() noexcept {
}

void hdr_dof_postprocess::set_z_buffer(const Core::Texture2D *z_buffer) {
	this->z_buffer = z_buffer;

	auto z_handle = z_buffer->get_texture_handle();
	z_handle.make_resident();

	hdr_tonemap_coc->set_uniform("z_buffer", z_handle);
	hdr_compute_histogram_sums->set_uniform("z_buffer", z_handle);
}

void hdr_dof_postprocess::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	bokeh_coc = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RG32_SFLOAT_PACK32, StE::Core::Texture2D::size_type(size), 1);

	hdr_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, StE::Core::Texture2D::size_type(size), 1);
	hdr_final_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, StE::Core::Texture2D::size_type(size), 1);
	hdr_bloom_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::Core::Texture2D::size_type(size), 1);

	fbo_hdr_final[0] = (*hdr_final_image)[0];
	fbo_hdr[0] = (*hdr_image)[0];
	fbo_hdr[1] = (*hdr_bloom_image)[0];
	fbo_hdr[2] = (*bokeh_coc)[0];

	hdr_bloom_blurx_image = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::Core::Texture2D::size_type(size), 1);

	fbo_hdr_bloom_blurx_image[0] = (*hdr_bloom_blurx_image)[0];

	luminance_size = size / 4;

	hdr_lums = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_SFLOAT_PACK32, StE::Core::Texture2D::size_type(luminance_size), 1);
	bokeh_blur_image_x = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGBA8_UNORM_PACK8, StE::Core::Texture2D::size_type(size), 1);
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
	
	hdr_bokeh_param_buffer_prev << *hdr_bokeh_param_buffer_eraser;
}

void hdr_dof_postprocess::set_context_state() const {
	Base::set_context_state();
		
	ScreenFillingQuad.vao()->bind();
	bokeh_blury->bind();
}

void hdr_dof_postprocess::dispatch() const {
	Core::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
