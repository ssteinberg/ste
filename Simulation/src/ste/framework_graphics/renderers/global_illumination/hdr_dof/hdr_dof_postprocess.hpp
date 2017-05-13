// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>

#include <device_image.hpp>
#include <array.hpp>
#include <framebuffer.hpp>
#include <combined_image_sampler.hpp>
#include <packaged_image_sampler.hpp>
#include <std430.hpp>

#include <hdr_bokeh_blur_fragment.hpp>
#include <hdr_bloom_blur_y_fragment.hpp>
#include <hdr_bloom_blur_x_fragment.hpp>
#include <hdr_tonemap_coc_fragment.hpp>
#include <hdr_compute_histogram_fragment.hpp>
#include <hdr_compute_histogram_sums_fragment.hpp>
#include <hdr_compute_minmax_fragment.hpp>

#include <command_recorder.hpp>

#include <signal.hpp>

#include <memory>

namespace ste {
namespace graphics {

class hdr_dof_postprocess : public gl::fragment {
private:
	struct hdr_bokeh_parameters : gl::std430<std::int32_t, std::int32_t, float> {
		using Base = gl::std430<std::int32_t, std::int32_t, float>;
		using Base::Base;

		auto& lum_min() { return get<0>(); }
		auto& lum_max() { return get<1>(); }
		auto& focus() { return get<2>(); }
	};

	static constexpr float vision_properties_max_lum = 10.f;
	static hdr_bokeh_parameters parameters_initial;

private:
	std::reference_wrapper<const ste_context> ctx;

	gl::device_image<2> hdr_final_image;
	gl::combined_image_sampler<gl::image_type::image_2d> hdr_final_linear;
	gl::combined_image_sampler<gl::image_type::image_2d> hdr_final_linear_clamp;

	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_bloom_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_bloom_blurx_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_lums;

	gl::packaged_image_sampler<gl::image_type::image_1d> hdr_vision_properties_texture;

	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer;
	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev;
	gl::array<gl::std430<std::uint32_t>> histogram;
	gl::array<gl::std430<std::uint32_t>> histogram_sums;

	gl::framebuffer fbo_hdr_final;
	gl::framebuffer fbo_hdr;
	gl::framebuffer fbo_hdr_bloom_blurx_image;

	hdr_compute_minmax_fragment compute_minmax_task;
	hdr_compute_histogram_fragment create_histogram_task;
	hdr_compute_histogram_sums_fragment compute_histogram_sums_task;
	hdr_tonemap_coc_fragment tonemap_coc_task;
	hdr_bloom_blur_x_fragment bloom_blurx_task;
	hdr_bloom_blur_y_fragment bloom_blury_task;
	hdr_bokeh_blur_fragment bokeh_blur_task;

private:
	static gl::packaged_image_sampler<gl::image_type::image_1d> create_hdr_vision_properties_texture(const ste_context &ctx);
	static gl::framebuffer_layout create_fb_layout(gl::format f);
	static gl::framebuffer_layout create_fb_layout(gl::format f1, gl::format f2);

	void bind_fragment_resources();

public:
	hdr_dof_postprocess(const gl::rendering_system &rs);

	auto &get_input_image() const { return hdr_final_image; }

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) const {
		assert(diameter > .0f && "Lens diameter must be positive");
		assert(focal_length > .0f && "Focal length must be positive");

//		bokeh_blur.get().set_uniform("aperture_diameter", diameter);
//		bokeh_blur.get().set_uniform("aperture_focal_length", focal_length);
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		bokeh_blur_task.attach_framebuffer(fb);
	}
	void record(gl::command_recorder &recorder) override final;

	void resize();

	auto& get_exposure_params_buffer() const { return hdr_bokeh_param_buffer; }
	auto& get_histogram_buffer() const { return histogram; }
	auto& get_histogram_sums_buffer() const { return histogram_sums; }
};

}
}
