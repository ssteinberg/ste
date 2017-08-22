// StE
// © Shlomi Steinberg, 2015-2017

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

	int depth_buffer_levels;
	glm::ivec2 extent;

	mutable signal<> gbuffer_resized_signal;

public:
	deferred_gbuffer(const ste_context &ctx,
					 const glm::ivec2 &extent,
					 int depth_buffer_levels);

	void resize(const glm::ivec2 &extent);

	void clear(gl::command_recorder &recorder) {
		glm::vec4 zero = { 0,0,0,0 };

		recorder << gl::cmd_clear_color_image(gbuffer,
											  gl::image_layout::transfer_dst_optimal,
											  zero);
	}

	auto& get_gbuffer() const { return gbuffer.get(); }

	auto& get_extent() const { return extent; }
	
	auto& get_depth_target() const { return depth_target.get(); }
	auto& get_backface_depth_target() const { return backface_depth_target.get(); }
	auto& get_downsampled_depth_target() const { return downsampled_depth_target.get(); }

	auto& get_gbuffer_level0() const { return gbuffer_level_0; }
	auto& get_gbuffer_level1() const { return gbuffer_level_1; }

	auto& get_depth_target_modified_signal() const { return gbuffer_resized_signal; }
};

}
}
