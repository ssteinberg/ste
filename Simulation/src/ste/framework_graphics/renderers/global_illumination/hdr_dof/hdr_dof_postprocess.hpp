// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>

#include <signal.hpp>

#include <gpu_task.hpp>
#include <Quad.hpp>

#include <image.hpp>
#include <texture_1d.hpp>
#include <texture_2d.hpp>
#include <shader_storage_buffer.hpp>
#include <pixel_buffer_object.hpp>
#include <atomic_counter_buffer_object.hpp>
#include <framebuffer_object.hpp>
#include <glsl_program.hpp>

#include <deferred_gbuffer.hpp>

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
class hdr_bokeh_blur_task;

class hdr_dof_postprocess {
	friend class Resource::resource_loading_task<hdr_dof_postprocess>;
	friend class Resource::resource_instance<hdr_dof_postprocess>;

	friend class hdr_compute_minmax_task;
	friend class hdr_create_histogram_task;
	friend class hdr_compute_histogram_sums_task;
	friend class hdr_tonemap_coc_task;
	friend class hdr_bloom_blurx_task;
	friend class hdr_bloom_blury_task;
	friend class hdr_bokeh_blur_task;

private:
	static constexpr float vision_properties_max_lum = 10.f;

private:
	using ResizeSignalConnectionType = ste_engine_control::framebuffer_resize_signal_type::connection_type;

	struct hdr_bokeh_parameters {
		std::int32_t lum_min, lum_max;
		float focus, _unused;
	};

private:
	const deferred_gbuffer *gbuffer;
	const ste_engine_control &ctx;

	std::shared_ptr<const gpu_task> task;

	std::unique_ptr<hdr_compute_minmax_task> compute_minmax_task;
	std::unique_ptr<hdr_create_histogram_task> create_histogram_task;
	std::unique_ptr<hdr_compute_histogram_sums_task> compute_histogram_sums_task;
	std::unique_ptr<hdr_tonemap_coc_task> tonemap_coc_task;
	std::unique_ptr<hdr_bloom_blurx_task> bloom_blurx_task;
	std::unique_ptr<hdr_bloom_blury_task> bloom_blury_task;
	std::unique_ptr<hdr_bokeh_blur_task> bokeh_blur_task;

	Resource::resource_instance<Resource::glsl_program> hdr_compute_minmax;
	Resource::resource_instance<Resource::glsl_program> hdr_create_histogram;
	Resource::resource_instance<Resource::glsl_program> hdr_compute_histogram_sums;
	Resource::resource_instance<Resource::glsl_program> hdr_tonemap_coc;
	Resource::resource_instance<Resource::glsl_program> hdr_bloom_blurx;
	Resource::resource_instance<Resource::glsl_program> hdr_bloom_blury;
	Resource::resource_instance<Resource::glsl_program> bokeh_blur;

	Core::sampler hdr_vision_properties_sampler;

	std::unique_ptr<Core::texture_1d> hdr_vision_properties_texture;
	Core::texture_handle hdr_vision_properties_texture_handle;

	std::unique_ptr<Core::texture_2d> hdr_image;
	std::unique_ptr<Core::texture_2d> hdr_final_image;
	std::unique_ptr<Core::texture_2d> hdr_bloom_image;
	std::unique_ptr<Core::texture_2d> hdr_bloom_blurx_image;
	std::unique_ptr<Core::texture_2d> hdr_lums;

	Core::framebuffer_object fbo_hdr_final;
	Core::framebuffer_object fbo_hdr;
	Core::framebuffer_object fbo_hdr_bloom_blurx_image;

	mutable Core::shader_storage_buffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer{ 1 };
	mutable Core::shader_storage_buffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev{ 1 };
	mutable Core::atomic_counter_buffer_object<> histogram{ 128 };
	mutable Core::shader_storage_buffer<std::uint32_t> histogram_sums{ 128 };
	mutable std::unique_ptr<Core::pixel_buffer_object<std::int32_t>> hdr_bokeh_param_buffer_eraser;

	glm::i32vec2 luminance_size;

	std::array<std::uint32_t, 4> storage_buffers;

private:
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<connection<>> gbuffer_depth_target_connection;

private:
	std::vector<std::shared_ptr<const gpu_task>> create_sub_tasks();
	hdr_bokeh_blur_task* create_dispatchable();

	void setup_connections();
	void attach_handles() const;

private:
	hdr_dof_postprocess(const ste_engine_control &ctx, const deferred_gbuffer *gbuffer);

public:
	~hdr_dof_postprocess() noexcept;

	auto &get_input_image() const { return *hdr_final_image; }
	auto get_input_fbo() const { return &fbo_hdr_final; }

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) const {
		assert(diameter > .0f && "Lens diameter must be positive");
		assert(focal_length > .0f && "Focal length must be positive");

		bokeh_blur.get().set_uniform("aperture_diameter", diameter);
		bokeh_blur.get().set_uniform("aperture_focal_length", focal_length);
	}

	void resize(glm::ivec2 size);

	std::shared_ptr<const gpu_task> get_task() const;
	auto& get_exposure_params_buffer() const { return hdr_bokeh_param_buffer; }
	auto& get_histogram_buffer() const { return histogram; }
	auto& get_histogram_sums_buffer() const { return histogram_sums; }
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::hdr_dof_postprocess> {
	using R = Graphics::hdr_dof_postprocess;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->hdr_compute_minmax.wait();
			object->hdr_create_histogram.wait();
			object->hdr_compute_histogram_sums.wait();
			object->hdr_tonemap_coc.wait();
			object->hdr_bloom_blurx.wait();
			object->hdr_bloom_blury.wait();
			object->bokeh_blur.wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object, &ctx]() {
			auto vision_handle = object->hdr_vision_properties_texture->get_texture_handle(object->hdr_vision_properties_sampler);
			vision_handle.make_resident();

			object->attach_handles();

			object->hdr_tonemap_coc.get().set_uniform("hdr_vision_properties_texture", vision_handle);
			object->hdr_bloom_blurx.get().set_uniform("dir", glm::vec2{ 1.f, .0f });
			object->hdr_bloom_blury.get().set_uniform("dir", glm::vec2{ .0f, 1.f });
			object->resize(ctx.get_backbuffer_size());
		});;
	}
};

}
}
