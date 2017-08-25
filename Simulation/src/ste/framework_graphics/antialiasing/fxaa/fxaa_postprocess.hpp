// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <rendering_system.hpp>
#include <fragment_graphics.hpp>

#include <texture.hpp>
#include <framebuffer.hpp>

namespace ste {
namespace graphics {

class fxaa_postprocess : public gl::fragment_graphics<fxaa_postprocess> {
	using Base = gl::fragment_graphics<fxaa_postprocess>;

private:
	alias<const ste_context> ctx;
	glm::u32vec2 extent;

	gl::task<gl::cmd_draw> draw_task;

	gl::pipeline_stage input_stage_flags;
	gl::image_layout input_image_layout;

public:
	fxaa_postprocess(const gl::rendering_system &rs,
					  gl::framebuffer_layout &&fb_layout);

	static const lib::string& name() { return "fxaa"; }

	/**
	*	@brief	Sets the input image.
	 *			The input image is expectd to be in layout image_layout::shader_read_only_optimal before rendering.
	 *			It is the caller's reponsibility to set a pipeline barrier if needed. Last access by the fragment is at pipeline stage pipeline_stage::fragment_shader.
	*
	*	@param	input				Input image
	*/
	void set_input_image(gl::texture<gl::image_type::image_2d> *input) {
		pipeline["input_tex"] = gl::bind(gl::pipeline::combined_image_sampler(*input,
																			  ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

}
}
