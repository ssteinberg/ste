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
	static auto create_hdr_input_fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::clear_store(gl::format::r16g16b16a16_sfloat,
									   gl::image_layout::shader_read_only_optimal);
		return fb_layout;
	}

	static auto create_fxaa_input_fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::ignore_store(gl::format::r8g8b8a8_unorm,
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
																								   extent)),
		fxaa_input_image(ctx,
						 resource::surface_factory::image_empty_2d<gl::format::r8g8b8a8_unorm>(ctx,
																							   gl::image_usage::sampled | gl::image_usage::color_attachment,
																							   gl::image_layout::shader_read_only_optimal,
																							   extent)),

		hdr_input_fb(ctx, create_hdr_input_fb_layout(ctx), extent),
		fxaa_input_fb(ctx, create_fxaa_input_fb_layout(ctx), extent)
	{
		hdr_input_fb[0] = gl::framebuffer_attachment(*hdr_input_image, glm::vec4(.0f));
		fxaa_input_fb[0] = gl::framebuffer_attachment(*fxaa_input_image, glm::vec4(.0f));
	}

	void resize(const glm::uvec2 &extent) {
		if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
			return;

		this->extent = extent;

		hdr_input_fb = gl::framebuffer(ctx.get(), create_hdr_input_fb_layout(ctx.get()), extent);
		fxaa_input_fb = gl::framebuffer(ctx.get(), create_fxaa_input_fb_layout(ctx.get()), extent);

		hdr_input_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx.get(),
																			  resource::surface_factory::image_empty_2d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																																						 gl::image_usage::sampled | gl::image_usage::color_attachment,
																																						 gl::image_layout::shader_read_only_optimal,
																																						 extent));
		fxaa_input_image = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx.get(),
																			   resource::surface_factory::image_empty_2d<gl::format::r8g8b8a8_unorm>(ctx.get(),
																																					 gl::image_usage::sampled | gl::image_usage::color_attachment,
																																					 gl::image_layout::shader_read_only_optimal,
																																					 extent));

		hdr_input_fb[0] = gl::framebuffer_attachment(*hdr_input_image, glm::vec4(.0f));
		fxaa_input_fb[0] = gl::framebuffer_attachment(*fxaa_input_image, glm::vec4(.0f));
	}
};

}
}
