// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "gpu_task.hpp"
#include "Quad.hpp"

#include "image.hpp"
#include "Texture1D.hpp"
#include "Texture2D.hpp"
#include "ShaderStorageBuffer.hpp"
#include "PixelBufferObject.hpp"
#include "AtomicCounterBufferObject.hpp"
#include "FramebufferObject.hpp"
#include "GLSLProgram.hpp"
#include "GLSLProgramFactory.hpp"

#include <memory>
#include <array>

namespace StE {
namespace Graphics {

class hdr_compute_minmax_task;
class hdr_create_histogram_task;
class hdr_compute_histogram_sums_task;
class hdr_tonemap_coc_task;
class hdr_bloom_blurx_task;
class hdr_bloom_blury_task;
class hdr_bokeh_blurx_task;
class hdr_bokeh_blury_task;

class hdr_dof_postprocess {
	friend class hdr_compute_minmax_task;
	friend class hdr_create_histogram_task;
	friend class hdr_compute_histogram_sums_task;
	friend class hdr_tonemap_coc_task;
	friend class hdr_bloom_blurx_task;
	friend class hdr_bloom_blury_task;
	friend class hdr_bokeh_blurx_task;
	friend class hdr_bokeh_blury_task;

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	struct hdr_bokeh_parameters {
		std::int32_t lum_min, lum_max;
		float focus;
	};

private:
	std::shared_ptr<const gpu_task> task;

	std::unique_ptr<hdr_compute_minmax_task> compute_minmax_task;
	std::unique_ptr<hdr_create_histogram_task> create_histogram_task;
	std::unique_ptr<hdr_compute_histogram_sums_task> compute_histogram_sums_task;
	std::unique_ptr<hdr_tonemap_coc_task> tonemap_coc_task;
	std::unique_ptr<hdr_bloom_blurx_task> bloom_blurx_task;
	std::unique_ptr<hdr_bloom_blury_task> bloom_blury_task;
	std::unique_ptr<hdr_bokeh_blurx_task> bokeh_blurx_task;
	std::unique_ptr<hdr_bokeh_blury_task> bokeh_blury_task;

	std::shared_ptr<Core::GLSLProgram> hdr_compute_minmax;
	std::shared_ptr<Core::GLSLProgram> hdr_create_histogram;
	std::shared_ptr<Core::GLSLProgram> hdr_compute_histogram_sums;
	std::shared_ptr<Core::GLSLProgram> hdr_tonemap_coc;
	std::shared_ptr<Core::GLSLProgram> hdr_bloom_blurx;
	std::shared_ptr<Core::GLSLProgram> hdr_bloom_blury;
	std::shared_ptr<Core::GLSLProgram> bokeh_blurx;
	std::shared_ptr<Core::GLSLProgram> bokeh_blury;

	Core::Sampler hdr_vision_properties_sampler;
	Core::Sampler linear_sampler;

	std::unique_ptr<Core::Texture1D> hdr_vision_properties_texture;
	Core::texture_handle hdr_vision_properties_texture_handle;

	std::unique_ptr<Core::Texture2D> bokeh_coc;
	std::unique_ptr<Core::Texture2D> hdr_image;
	std::unique_ptr<Core::Texture2D> hdr_final_image;
	std::unique_ptr<Core::Texture2D> hdr_bloom_image;
	std::unique_ptr<Core::Texture2D> hdr_bloom_blurx_image;
	std::unique_ptr<Core::Texture2D> hdr_lums;
	std::unique_ptr<Core::Texture2D> bokeh_blur_image_x;

	Core::FramebufferObject fbo_hdr_final;
	Core::FramebufferObject fbo_hdr;
	Core::FramebufferObject fbo_hdr_bloom_blurx_image;
	Core::FramebufferObject fbo_bokeh_blur_image;

	mutable Core::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer{ 1 };
	mutable Core::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev{ 1 };
	mutable Core::AtomicCounterBufferObject<> histogram{ 128 };
	mutable Core::ShaderStorageBuffer<std::uint32_t> histogram_sums{ 128 };
	mutable std::unique_ptr<Core::PixelBufferObject<std::int32_t>> hdr_bokeh_param_buffer_eraser;

	glm::i32vec2 luminance_size;

	const Core::Texture2D *z_buffer;
	const StEngineControl &ctx;

	std::array<std::uint32_t, 4> storage_buffers;

private:
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

private:
	std::vector<std::shared_ptr<const gpu_task>> create_sub_tasks();
	hdr_bokeh_blury_task* create_dispatchable();

public:
	hdr_dof_postprocess(const StEngineControl &ctx, const Core::Texture2D *z_buffer);
	~hdr_dof_postprocess() noexcept;

	void set_z_buffer(const Core::Texture2D *z_buffer);

	auto get_input_fbo() const { return &fbo_hdr_final; }

	void resize(glm::ivec2 size);

	std::shared_ptr<const gpu_task> get_task() const;
};

}
}
