
#include <stdafx.hpp>
#include <deferred_gbuffer.hpp>

#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

//gl::framebuffer_layout deferred_gbuffer::create_fbo_layout() {
//	gl::framebuffer_layout fb_layout;
//	fb_layout[0] = gl::ignore_store(gl::format::r32g32b32a32_sfloat,
//									gl::image_layout::color_attachment_optimal);
//	fb_layout[1] = gl::ignore_store(gl::format::r32g32b32a32_sfloat,
//									gl::image_layout::color_attachment_optimal);
//	fb_layout[gl::pipeline_depth_attachment_location] = gl::ignore_store(gl::format::d32_sfloat,
//																		 gl::image_layout::depth_stencil_attachment_optimal);
//	return fb_layout;
//}
//
//gl::framebuffer_layout deferred_gbuffer::create_backface_fbo_layout() {
//	gl::framebuffer_layout fb_layout;
//	fb_layout[gl::pipeline_depth_attachment_location] = gl::ignore_store(gl::format::d32_sfloat,
//																		 gl::image_layout::depth_stencil_attachment_optimal);
//	return fb_layout;
//}

deferred_gbuffer::deferred_gbuffer(const ste_context &ctx,
								   const glm::ivec2 &extent,
								   int depth_buffer_levels)
	: ctx(ctx),
	depth_target(ctx,
				 resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																				   gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																				   gl::image_layout::depth_stencil_attachment_optimal,
																				   extent)),
	backface_depth_target(ctx,
						  resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																							gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																							gl::image_layout::depth_stencil_attachment_optimal,
																							extent)),
	downsampled_depth_target(ctx,
							 resource::surface_factory::image_empty_2d<gl::format::r32g32_sfloat>(ctx,
																								  gl::image_usage::sampled | gl::image_usage::color_attachment,
																								  gl::image_layout::color_attachment_optimal,
																								  extent / 2, 1, depth_buffer_levels - 1)),
	gbuffer(resource::surface_factory::image_empty_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																					   gl::image_usage::sampled | gl::image_usage::color_attachment,
																					   gl::image_layout::color_attachment_optimal,
																					   extent, 2).get()),
	gbuffer_level_0(gbuffer, gbuffer.get_format(), 0, 1),
	gbuffer_level_1(gbuffer, gbuffer.get_format(), 1, 1),
//	fbo(ctx, create_fbo_layout(), extent),
//	backface_fbo(ctx, create_backface_fbo_layout(), extent),
	extent(extent),
	depth_buffer_levels(depth_buffer_levels) 
{
//	fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get());
//	fbo[0] = gl::framebuffer_attachment(gbuffer_level_0);
//	fbo[1] = gl::framebuffer_attachment(gbuffer_level_1);
//	backface_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(backface_depth_target.get());
}

void deferred_gbuffer::resize(const glm::ivec2 &extent) {
	if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
		return;

	this->extent = extent;

	depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																	   resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																																		 gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																		 gl::image_layout::depth_stencil_attachment_optimal,
																																		 extent));
	backface_depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																				resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																																				  gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																				  gl::image_layout::depth_stencil_attachment_optimal,
																																				  extent));
	downsampled_depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																				   resource::surface_factory::image_empty_2d<gl::format::r32g32_sfloat>(ctx,
																																						gl::image_usage::sampled | gl::image_usage::color_attachment,
																																						gl::image_layout::color_attachment_optimal,
																																						extent / 2, 1, depth_buffer_levels - 1));
	gbuffer = resource::surface_factory::image_empty_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																						 gl::image_usage::sampled | gl::image_usage::color_attachment,
																						 gl::image_layout::color_attachment_optimal,
																						 extent, 2).get();
	gbuffer_level_0 = gl::image_view<gl::image_type::image_2d>(gbuffer, gbuffer.get_format(), 0, 1);
	gbuffer_level_1 = gl::image_view<gl::image_type::image_2d>(gbuffer, gbuffer.get_format(), 1, 1);

//	fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get());
//	fbo[0] = gl::framebuffer_attachment(gbuffer_level_0);
//	fbo[1] = gl::framebuffer_attachment(gbuffer_level_1);
//	backface_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(backface_depth_target.get());

	gbuffer_resized_signal.emit();
}
