// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <signal.hpp>

#include <ste_resource.hpp>
#include <image_type.hpp>
#include <texture.hpp>
#include <framebuffer.hpp>

#include <command_recorder.hpp>
#include <cmd_clear_color_image.hpp>

namespace ste {
namespace graphics {

class deferred_gbuffer {
private:
	alias<const ste_context> ctx;

	ste_resource<gl::texture<gl::image_type::image_2d>> depth_target;
	ste_resource<gl::texture<gl::image_type::image_2d>> backface_depth_target;
	ste_resource<gl::texture<gl::image_type::image_2d>> downsampled_depth_target;

	gl::device_image<2> gbuffer;
	gl::image_view<gl::image_type::image_2d> gbuffer_level_0;
	gl::image_view<gl::image_type::image_2d> gbuffer_level_1;

//	gl::framebuffer fbo;
//	gl::framebuffer backface_fbo;

	glm::ivec2 size;
	int depth_buffer_levels;

	mutable signal<> gbuffer_resized_signal;

//private:
//	static gl::framebuffer_layout create_fbo_layout();
//	static gl::framebuffer_layout create_backface_fbo_layout();

public:
	deferred_gbuffer(const ste_context &ctx,
					 glm::ivec2 size, 
					 int depth_buffer_levels);

	void resize(glm::ivec2 size);

	void clear(gl::command_recorder &recorder) {
		glm::vec4 zero = { 0,0,0,0 };

		recorder << gl::cmd_clear_color_image(gbuffer,
											  gl::image_layout::transfer_dst_optimal,
											  zero);
	}

	auto& get_gbuffer() const { return gbuffer.get(); }

	auto& get_size() const { return size; }
	
	auto& get_depth_target() const { return depth_target.get(); }
	auto& get_backface_depth_target() const { return backface_depth_target.get(); }
	auto& get_downsampled_depth_target() const { return downsampled_depth_target.get(); }
//	auto* get_fbo() const { return &fbo; }
//	auto* get_backface_fbo() const { return &backface_fbo; }

	auto& get_gbuffer_level0() const { return gbuffer_level_0; }
	auto& get_gbuffer_level1() const { return gbuffer_level_1; }

	auto& get_depth_target_modified_signal() const { return gbuffer_resized_signal; }
};

}
}
