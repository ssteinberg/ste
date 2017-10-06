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

	levels_t depth_buffer_levels;

	ste_resource<gl::texture<gl::image_type::image_2d>> depth_target;
	ste_resource<gl::texture<gl::image_type::image_2d>> backface_depth_target;
	ste_resource<gl::texture<gl::image_type::image_2d>> downsampled_depth_target;

	gl::texture<gl::image_type::image_2d_array> gbuffer;
	gl::image_view<gl::image_type::image_2d> gbuffer_level_0;
	gl::image_view<gl::image_type::image_2d> gbuffer_level_1;

	gl::framebuffer fbo, depth_fbo, depth_backface_fbo;
	glm::uvec2 extent;

	mutable signal<> gbuffer_resized_signal;

private:
	gl::framebuffer_layout create_fbo_layout();
	gl::framebuffer_layout create_depth_fbo_layout();

public:
	deferred_gbuffer(const ste_context &ctx,
					 const glm::uvec2 &extent,
					 levels_t depth_buffer_levels);
	~deferred_gbuffer() noexcept {}

	deferred_gbuffer(deferred_gbuffer&&) = default;

	void resize(const glm::uvec2 &extent);

	void clear(gl::command_recorder &recorder) {
		const glm::vec4 zero = { 0,0,0,0 };

		recorder << gl::cmd_clear_color_image(gbuffer.get_image(),
											  gl::image_layout::transfer_dst_optimal,
											  zero);
	}

	auto& get_gbuffer() const { return gbuffer; }

	auto& get_extent() const { return extent; }
	
	auto& get_depth_target() const { return depth_target.get(); }
	auto& get_backface_depth_target() const { return backface_depth_target.get(); }
	auto& get_downsampled_depth_target() const { return downsampled_depth_target.get(); }

	auto& get_gbuffer_level0() const { return gbuffer_level_0; }
	auto& get_gbuffer_level1() const { return gbuffer_level_1; }

	auto& get_fbo() { return fbo; }
	auto& get_depth_fbo() { return depth_fbo; }
	auto& get_depth_backface_fbo() { return depth_backface_fbo; }

	auto& get_gbuffer_modified_signal() const { return gbuffer_resized_signal; }
};

}
}
