
#include <stdafx.hpp>
#include <deferred_gbuffer.hpp>

#include <surface_factory.hpp>

#include <scene_write_gbuffer_fragment.hpp>
#include <scene_prepopulate_depth_fragment.hpp>

using namespace ste;
using namespace ste::graphics;

gl::framebuffer_layout deferred_gbuffer::create_fbo_layout() {
	return scene_write_gbuffer_fragment::create_fb_layout();
}

gl::framebuffer_layout deferred_gbuffer::create_depth_fbo_layout() {
	return scene_prepopulate_depth_fragment<true>::create_fb_layout();
}

deferred_gbuffer::deferred_gbuffer(const ste_context &ctx,
								   const glm::uvec2 &extent,
								   int depth_buffer_levels)
	: ctx(ctx),
	depth_target(ctx,
				 resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																				   gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																				   gl::image_layout::shader_read_only_optimal,
																				   "gbuffer depth target",
																				   extent)),
	backface_depth_target(ctx,
						  resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																							gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																							gl::image_layout::shader_read_only_optimal,
																							"gbuffer back-face depth target",
																							extent)),
	downsampled_depth_target(ctx,
							 resource::surface_factory::image_empty_2d<gl::format::r32g32_sfloat>(ctx,
																								  gl::image_usage::sampled | gl::image_usage::storage,
																								  gl::image_layout::shader_read_only_optimal,
																								  "gbuffer downsampled depth target",
																								  extent / 2u, 1, depth_buffer_levels - 1)),
	gbuffer(resource::surface_factory::image_empty_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																					   gl::image_usage::sampled | gl::image_usage::color_attachment,
																					   gl::image_layout::color_attachment_optimal,
																					   "gbuffer",
																					   extent, 2).get()),
	gbuffer_level_0(gbuffer.get_image(), 
					gbuffer->get_format(), 
					0, 1),
	gbuffer_level_1(gbuffer.get_image(), 
					gbuffer->get_format(), 
					1, 1),
	fbo(ctx, 
		"gbuffer framebuffer",
		create_fbo_layout(), 
		extent),
	depth_fbo(ctx,
			  "gbuffer depth framebuffer", 
			  create_depth_fbo_layout(), 
			  extent),
	depth_backface_fbo(ctx,
					   "gbuffer back-face depth framebuffer", 
					   create_depth_fbo_layout(), 
					   extent),
	depth_buffer_levels(depth_buffer_levels),
	extent(extent)
{
	fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get(), glm::vec4(.0f));
	fbo[0] = gl::framebuffer_attachment(gbuffer_level_0);
	fbo[1] = gl::framebuffer_attachment(gbuffer_level_1);

	depth_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get(), glm::vec4(.0f));
	depth_backface_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(backface_depth_target.get(), glm::vec4(.0f));
}

void deferred_gbuffer::resize(const glm::uvec2 &extent) {
	if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
		return;

	this->extent = extent;

	// Recreate images
	depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																	   resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																																		 gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																		 gl::image_layout::shader_read_only_optimal,
																																		 "gbuffer depth target",
																																		 extent));
	backface_depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																				resource::surface_factory::image_empty_2d<gl::format::d32_sfloat>(ctx,
																																				  gl::image_usage::sampled | gl::image_usage::depth_stencil_attachment,
																																				  gl::image_layout::shader_read_only_optimal,
																																				  "gbuffer back-face depth target",
																																				  extent));
	downsampled_depth_target = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																				   resource::surface_factory::image_empty_2d<gl::format::r32g32_sfloat>(ctx,
																																						gl::image_usage::sampled | gl::image_usage::storage,
																																						gl::image_layout::shader_read_only_optimal,
																																						"gbuffer downsampled depth target",
																																						extent / 2u, 1, depth_buffer_levels - 1));
	gbuffer = resource::surface_factory::image_empty_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																						 gl::image_usage::sampled | gl::image_usage::color_attachment,
																						 gl::image_layout::color_attachment_optimal,
																						 "gbuffer",
																						 extent, 2).get();
	gbuffer_level_0 = gl::image_view<gl::image_type::image_2d>(gbuffer.get_image(), 
															   gbuffer->get_format(), 
															   0, 1);
	gbuffer_level_1 = gl::image_view<gl::image_type::image_2d>(gbuffer.get_image(), 
															   gbuffer->get_format(), 
															   1, 1);

	// Recreate framebuffers
	fbo = gl::framebuffer(ctx,
						  "gbuffer framebuffer",
						  create_fbo_layout(),
						  extent);
	depth_fbo = gl::framebuffer(ctx,
								"gbuffer depth framebuffer",
								create_depth_fbo_layout(),
								extent);
	depth_backface_fbo = gl::framebuffer(ctx,
										 "gbuffer back-face depth framebuffer",
										 create_depth_fbo_layout(),
										 extent);

	// Reattach framebuffer attachments
	fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get(), glm::vec4(.0f));
	fbo[0] = gl::framebuffer_attachment(gbuffer_level_0);
	fbo[1] = gl::framebuffer_attachment(gbuffer_level_1);

	depth_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(depth_target.get(), glm::vec4(.0f));
	depth_backface_fbo[gl::pipeline_depth_attachment_location] = gl::framebuffer_attachment(backface_depth_target.get(), glm::vec4(.0f));

	gbuffer_resized_signal.emit();
}
