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
#include <texture.hpp>
#include <std430.hpp>

#include <hdr_bokeh_blur_fragment.hpp>
#include <hdr_bloom_blur_y_fragment.hpp>
#include <hdr_bloom_blur_x_fragment.hpp>
#include <hdr_tonemap_coc_fragment.hpp>
#include <hdr_compute_histogram_fragment.hpp>
#include <hdr_compute_histogram_sums_fragment.hpp>
#include <hdr_compute_minmax_fragment.hpp>

#include <command_recorder.hpp>

#include <alias.hpp>

namespace ste {
namespace graphics {

class hdr_dof_postprocess : public gl::fragment {
private:
	static constexpr float vision_properties_max_lum = 10.f;
	static hdr_bokeh_parameters parameters_initial;

	static constexpr float default_aperature_diameter = 8e-3f;
	static constexpr float default_aperature_focal_ln = 23e-3f;

private:
	alias<const ste_context> ctx;
	glm::u32vec2 extent;

	gl::pipeline_stage input_stage_flags;
	gl::image_layout input_image_layout;

	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_final_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_bloom_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_bloom_blurx_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_lums;

	ste_resource<gl::texture<gl::image_type::image_1d, 2>> hdr_vision_properties_texture;

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

public:
	static constexpr auto input_image_format = gl::format::r16g16b16a16_sfloat;

private:
	static ste_resource<gl::texture<gl::image_type::image_1d, 2>> create_hdr_vision_properties_texture(const ste_context &ctx);

	void bind_fragment_resources();

public:
	hdr_dof_postprocess(const gl::rendering_system &rs,
						const glm::u32vec2 &extent,
						gl::framebuffer_layout &&fb_layout);

	/**
	 *	@brief	Returns the input image. 
	 *			Before writing to the image, it is the caller's reponsibility to set a pipeline barrier. The input image is left at image layout image_layout::shader_read_only_optimal,
	 *			accessed at pipeline stage pipeline_stage::fragment_shader.
	 *			
	 *			Likewise, the caller must specify its access type and layout of the image.
	 *	
	 *	@param	input_stage_flags	Consumer's last pipeline access stage
	 *	@param	input_image_layout	Consumer's last image layout
	 */
	auto &acquire_input_image(gl::pipeline_stage input_stage_flags,
							  gl::image_layout input_image_layout) {
		this->input_stage_flags = input_stage_flags;
		this->input_image_layout = input_image_layout;

		return hdr_final_image.get();
	}

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

	void resize(const glm::u32vec2 &extent);
};

}
}
