
#include <stdafx.hpp>
#include <primary_renderer.hpp>

#include <pipeline_external_binding_set_layout.hpp>

using namespace ste;
using namespace ste::graphics;

gl::framebuffer_layout primary_renderer::create_fb_layout(const ste_context &ctx) {
	gl::framebuffer_layout fb_layout;
	fb_layout[0] = gl::ignore_store(ctx.device().get_surface().surface_format(),
									gl::image_layout::color_attachment_optimal);
	return fb_layout;
}

primary_renderer::primary_renderer(const ste_context &ctx,
								   gl::presentation_engine &presentation,
								   const camera<float> *cam,
								   scene *s,
								   const atmospherics_properties<double> &atmospherics_prop)
	: Base(ctx,
		   create_fb_layout(ctx)), 
	presentation(presentation),
	cam(cam),
	s(s),

	transform_buffers(ctx),
	atmospheric_buffer(atmospherics_prop),

	lll_storage(ctx, ctx.device().get_surface().extent()),
	shadows_storage(ctx),
	vol_scat_storage(ctx),

	composer(ctx),
	fxaa(ctx),
	hdr(ctx),

	downsample_depth(ctx),
	prepopulate_depth_dispatch(ctx),
	prepopulate_backface_depth_dispatch(ctx),
	scene_geo_cull(ctx),

	lll_gen_dispatch(ctx),
	light_preprocess(ctx),

	shadows_projector(ctx),
	directional_shadows_projector(ctx),

	vol_scat_scatter(ctx) 
{
	lib::vector<gl::pipeline_external_binding_set_layout> common_binding_set_layouts;
	common_binding_set_collection = lib::allocate_unique<gl::pipeline_external_binding_set_collection>();
}
