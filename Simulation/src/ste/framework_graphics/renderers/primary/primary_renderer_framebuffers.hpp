// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <ste_resource.hpp>
#include <image_type.hpp>
#include <texture.hpp>
#include <framebuffer.hpp>

#include <surface_factory.hpp>

#include <deferred_composer.hpp>

namespace ste {
namespace graphics {

class primary_renderer_framebuffers {
	friend class primary_renderer;

private:
	alias<const ste_context> ctx;
	glm::uvec2 extent;

	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_input_image;
	ste_resource<gl::texture<gl::image_type::image_2d>> fxaa_input_image;

	gl::framebuffer hdr_input_fb, fxaa_input_fb;

private:
	static auto create_hdr_input_fb_layout() {
		return deferred_composer::create_fb_layout();
	}

	static auto create_fxaa_input_fb_layout() {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::ignore_store(gl::format::r16g16b16a16_sfloat,
										gl::image_layout::shader_read_only_optimal);
		return fb_layout;
	}

public:
	primary_renderer_framebuffers(const ste_context &ctx,
								  const glm::uvec2 &extent)
		: ctx(ctx),
		  extent(extent),

		  hdr_input_image(ctx,
						  resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx,
																									 gl::image_usage::sampled | gl::image_usage::color_attachment,
																									 gl::image_layout::shader_read_only_optimal,
																									 "hdr_input_image",
																									 extent)),
		  fxaa_input_image(ctx,
						   resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx,
																									  gl::image_usage::sampled | gl::image_usage::color_attachment,
																									  gl::image_layout::shader_read_only_optimal,
																									  "fxaa_input_image",
																									  extent)),

		  hdr_input_fb(ctx,
					   "hdr_input_fb",
					   create_hdr_input_fb_layout(),
					   extent),
		  fxaa_input_fb(ctx,
						"fxaa_input_fb",
						create_fxaa_input_fb_layout(),
						extent) {
		hdr_input_fb[0] = gl::framebuffer_attachment(*hdr_input_image, glm::vec4(.0f));
		fxaa_input_fb[0] = gl::framebuffer_attachment(*fxaa_input_image, glm::vec4(.0f));
	}

	~primary_renderer_framebuffers() noexcept {}

	primary_renderer_framebuffers(primary_renderer_framebuffers &&) = default;

	void resize(const glm::uvec2 &extent) {
		if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
			return;

		this->extent = extent;

		// Recreate framebuffers
		hdr_input_fb = gl::framebuffer(ctx.get(),
									   "hdr_input_fb",
									   create_hdr_input_fb_layout(),
									   extent);
		fxaa_input_fb = gl::framebuffer(ctx.get(),
										"fxaa_input_fb",
										create_fxaa_input_fb_layout(),
										extent);

		// Recreate images
		hdr_input_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx.get(),
																			  resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																																						 gl::image_usage::sampled | gl::image_usage::color_attachment,
																																						 gl::image_layout::shader_read_only_optimal,
																																						 "hdr_input_image",
																																						 extent));
		fxaa_input_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx.get(),
																			   resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																																						  gl::image_usage::sampled | gl::image_usage::color_attachment,
																																						  gl::image_layout::shader_read_only_optimal,
																																						  "fxaa_input_image",
																																						  extent));

		// Reattach framebuffer attachments
		hdr_input_fb[0] = gl::framebuffer_attachment(*hdr_input_image, glm::vec4(.0f));
		fxaa_input_fb[0] = gl::framebuffer_attachment(*fxaa_input_image, glm::vec4(.0f));
	}
};

}
}
