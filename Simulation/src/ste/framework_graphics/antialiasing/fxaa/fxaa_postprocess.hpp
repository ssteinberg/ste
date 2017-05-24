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

	ste_resource<gl::texture<gl::image_type::image_2d>> input_image;

public:
	static constexpr auto input_image_format = gl::format::r8g8b8a8_unorm;

public:
	fxaa_postprocess(const gl::rendering_system &rs,
					  const glm::u32vec2 &extent,
					  gl::framebuffer_layout &&fb_layout);

	static const lib::string& name() { return "fxaa"; }

	void resize(const glm::u32vec2 &extent);

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

		return input_image.get();
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
