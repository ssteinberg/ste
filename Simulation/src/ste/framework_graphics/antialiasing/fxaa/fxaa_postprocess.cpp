
#include <stdafx.hpp>
#include <fxaa_postprocess.hpp>

#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

fxaa_postprocess::fxaa_postprocess(const gl::rendering_system &rs,
								   gl::framebuffer_layout &&fb_layout)
	: Base(rs,
		   gl::device_pipeline_graphics_configurations{},
		   std::move(fb_layout),
		   "fullscreen_triangle.vert", "fxaa.frag"),
	ctx(rs.get_creating_context())
{
	draw_task.attach_pipeline(pipeline());
}
