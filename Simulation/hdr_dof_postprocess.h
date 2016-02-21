// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "renderable.h"
#include "Quad.h"

#include "Texture1D.h"
#include "Texture2D.h"
#include "ShaderStorageBuffer.h"
#include "PixelBufferObject.h"
#include "AtomicCounterBufferObject.h"
#include "FramebufferObject.h"
#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"

#include <memory>
#include <array>

namespace StE {
namespace Graphics {

class hdr_dof_postprocess : public renderable {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	struct hdr_bokeh_parameters {
		std::int32_t lum_min, lum_max;
		float focus;
	};

private:
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

	std::unique_ptr<LLR::GLSLProgram> hdr_compute_minmax;
	std::unique_ptr<LLR::GLSLProgram> hdr_create_histogram;
	std::unique_ptr<LLR::GLSLProgram> hdr_compute_histogram_sums;
	std::unique_ptr<LLR::GLSLProgram> hdr_tonemap_coc;
	std::unique_ptr<LLR::GLSLProgram> hdr_bloom_blurx;
	std::unique_ptr<LLR::GLSLProgram> hdr_bloom_blury;
	std::unique_ptr<LLR::GLSLProgram> bokeh_blurx;
	std::unique_ptr<LLR::GLSLProgram> bokeh_blury;

	LLR::Sampler hdr_vision_properties_sampler;
	LLR::Sampler linear_sampler;

	std::unique_ptr<LLR::Texture1D> hdr_vision_properties_texture;
	LLR::texture_handle hdr_vision_properties_texture_handle;

	std::unique_ptr<LLR::Texture2D> bokeh_coc;
	std::unique_ptr<LLR::Texture2D> hdr_image;
	std::unique_ptr<LLR::Texture2D> hdr_final_image;
	std::unique_ptr<LLR::Texture2D> hdr_bloom_image;
	std::unique_ptr<LLR::Texture2D> hdr_bloom_blurx_image;
	std::unique_ptr<LLR::Texture2D> hdr_lums;
	std::unique_ptr<LLR::Texture2D> bokeh_blur_image_x;

	LLR::FramebufferObject fbo_hdr_final;
	LLR::FramebufferObject fbo_hdr;
	LLR::FramebufferObject fbo_hdr_bloom_blurx_image;
	LLR::FramebufferObject fbo_bokeh_blur_image;

	mutable LLR::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer{ 1 };
	mutable LLR::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev{ 1 };
	mutable LLR::AtomicCounterBufferObject<> histogram{ 128 };
	mutable LLR::ShaderStorageBuffer<unsigned> histogram_sums{ 128 };
	mutable std::unique_ptr<LLR::PixelBufferObject<std::int32_t>> hdr_bokeh_param_buffer_eraser;

	mutable bool first_frame{ true };
	glm::i32vec2 luminance_size;

	const LLR::Texture2D *z_buffer;
	const StEngineControl &ctx;

	std::array<unsigned, 4> storage_buffers;

public:
	hdr_dof_postprocess(const StEngineControl &ctx, const LLR::Texture2D *z_buffer);
	~hdr_dof_postprocess() noexcept {}

	virtual void prepare() const override;
	virtual void render() const override;

	void set_z_buffer(const LLR::Texture2D *z_buffer);

	auto get_input_fbo() const { return &fbo_hdr_final; }

	void resize(glm::ivec2 size);
};

}
}
