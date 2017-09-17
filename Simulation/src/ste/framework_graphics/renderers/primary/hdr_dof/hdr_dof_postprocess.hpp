// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>

#include <framebuffer.hpp>
#include <texture.hpp>

#include <hdr_dof_postprocess_storage.hpp>
#include <hdr_bokeh_blur_fragment.hpp>
#include <hdr_bloom_blur_y_fragment.hpp>
#include <hdr_bloom_blur_x_fragment.hpp>
#include <hdr_tonemap_coc_fragment.hpp>
#include <hdr_compute_histogram_fragment.hpp>
#include <hdr_compute_histogram_sums_fragment.hpp>
#include <hdr_adaptation_fragment.hpp>
#include <hdr_compute_minmax_fragment.hpp>

#include <command_recorder.hpp>

#include <alias.hpp>

namespace ste {
namespace graphics {

class hdr_dof_postprocess : public gl::fragment {
private:
	static constexpr float default_aperature_diameter = 8e-3f;
	static constexpr float default_aperature_focal_ln = 23e-3f;

private:
	alias<const ste_context> ctx;
	glm::u32vec2 extent;
	gl::rendering_system::storage_ptr<hdr_dof_postprocess_storage> s;

	gl::texture<gl::image_type::image_2d> *input{ nullptr };
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_bloom_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_bloom_blurx_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_lums;

	hdr_compute_minmax_fragment compute_minmax_task;
	hdr_compute_histogram_fragment create_histogram_task;
	hdr_adaptation_fragment adaptation_task;
	hdr_compute_histogram_sums_fragment compute_histogram_sums_task;
	hdr_tonemap_coc_fragment tonemap_coc_task;
	hdr_bloom_blur_x_fragment bloom_blurx_task;
	hdr_bloom_blur_y_fragment bloom_blury_task;
	hdr_bokeh_blur_fragment bokeh_blur_task;

	gl::framebuffer fbo_hdr_final;
	gl::framebuffer fbo_hdr;
	gl::framebuffer fbo_hdr_bloom_blurx_image;

//	float tick_time_ms{ .0f };
	bool invalidated{ true };

private:
	void bind_fragment_resources();

public:
	hdr_dof_postprocess(gl::rendering_system &rs,
						const glm::u32vec2 &extent,
						gl::framebuffer_layout &&fb_layout);
	~hdr_dof_postprocess() noexcept {}

	hdr_dof_postprocess(hdr_dof_postprocess&&o) = default;


	static lib::string name() { return "hdr"; }

	/**
	 *	@brief	Sets the input image. 
	 *			The input image is expectd to be in layout image_layout::shader_read_only_optimal before rendering.
	 *			It is the caller's reponsibility to set a pipeline barrier if needed. Last access by the fragment is at pipeline stage pipeline_stage::fragment_shader.
	 *			
	 *			Expectes a RGBA floating point format.
	 *	
	 *	@param	input				Input image
	 */
	void set_input_image(gl::texture<gl::image_type::image_2d> *input) {
		this->input = input;
		this->invalidated = true;
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
