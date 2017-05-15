// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

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

namespace ste {
namespace graphics {

class hdr_dof_postprocess : public gl::fragment {
private:
	static constexpr float vision_properties_max_lum = 10.f;
	static hdr_bokeh_parameters parameters_initial;

	static constexpr float default_aperature_diameter = 8e-3f;
	static constexpr float default_aperature_focal_ln = 23e-3f;

private:
	std::reference_wrapper<const ste_context> ctx;

	gl::pipeline_stage input_stage_flags;
	gl::image_layout input_image_layout;

	gl::device_image<2> hdr_final_image;
	gl::combined_image_sampler<gl::image_type::image_2d> hdr_final_linear;
	gl::combined_image_sampler<gl::image_type::image_2d> hdr_final_linear_clamp;

	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_bloom_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_bloom_blurx_image;
	gl::packaged_image_sampler<gl::image_type::image_2d> hdr_lums;

	gl::device_image<2> hdr_vision_properties_image;
	gl::combined_image_sampler<gl::image_type::image_1d> hdr_vision_properties_texture;

	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer;
//	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev;
	gl::array<gl::std430<std::uint32_t>> histogram;
	gl::array<gl::std430<std::uint32_t>> histogram_sums;

	hdr_compute_minmax_fragment compute_minmax_task;
	hdr_compute_histogram_fragment create_histogram_task;
	hdr_compute_histogram_sums_fragment compute_histogram_sums_task;
	hdr_tonemap_coc_fragment tonemap_coc_task;
	hdr_bloom_blur_x_fragment bloom_blurx_task;
	hdr_bloom_blur_y_fragment bloom_blury_task;
	hdr_bokeh_blur_fragment bokeh_blur_task;

	gl::framebuffer fbo_hdr_final;
	gl::framebuffer fbo_hdr;
	gl::framebuffer fbo_hdr_bloom_blurx_image;

private:
	static gl::device_image<2> create_hdr_vision_properties_texture(const ste_context &ctx);

	void bind_fragment_resources();

public:
	hdr_dof_postprocess(const gl::rendering_system &rs,
						gl::framebuffer_layout &&fb_layout);

	/**
	 *	@brief	Returns the source image. Before writing to the image, the caller should transform the image from layout gl::image_layout::shader_read_only_optimal at stage 
	 *			gl::pipeline_stage::fragment_shader.
	 *			The caller specifies its access type and layout of the image.
	 *	
	 *	@param	input_stage_flags	Pipeline access stage
	 */
	auto &acquire_input_image(gl::pipeline_stage input_stage_flags,
							  gl::image_layout input_image_layout) {
		this->input_stage_flags = input_stage_flags;
		this->input_image_layout = input_image_layout;

		return hdr_final_image;
	}
	static auto input_image_format() { return gl::format::r16g16b16a16_sfloat; }

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) {
		assert(diameter > .0f && "Lens diameter must be positive");
		assert(focal_length > .0f && "Focal length must be positive");

		bokeh_blur_task.set_aperture_parameters(diameter, focal_length);
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		bokeh_blur_task.attach_framebuffer(fb);
	}
	void record(gl::command_recorder &recorder) override final;

	void resize();
};

}
}
