// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <device_pipeline_graphics.hpp>
#include <command_recorder.hpp>

#include <font.hpp>
#include <glyph_manager.hpp>
#include <glyph_point.hpp>

#include <attributed_string.hpp>

#include <memory>
#include <vector>

namespace ste {
namespace text {

/**
 *	@brief	Controls glyph loading, storage and rendering. Generates text rendering fragments.
 *			Provides a single device pipeline and framebuffer layout for all fragments created using this manager. 
 */
class text_manager : ste_resource_deferred_create_trait {
private:
	friend class text_fragment;

private:
	std::reference_wrapper<const ste_context> context;

	glyph_manager gm;
	font default_font;
	int default_size;

	ste_resource<gl::device_pipeline_shader_stage> vert;
	ste_resource<gl::device_pipeline_shader_stage> geom;
	ste_resource<gl::device_pipeline_shader_stage> frag;
	gl::device_pipeline_graphics pipeline;

	gl::ste_device::queues_and_surface_recreate_signal_type::connection_type surface_recreate_signal_connection;

private:
	static void adjust_line(std::vector<glyph_point> &, const attributed_wstring &, unsigned, float, float, const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2, const attributed_wstring &);

private:
	static gl::device_pipeline_graphics create_pipeline(const ste_context &,
														gl::device_pipeline_shader_stage &,
														gl::device_pipeline_shader_stage &,
														gl::device_pipeline_shader_stage &);
	void bind_pipeline_resources(std::uint32_t);
	void recreate_pipeline();
	bool update_glyphs(gl::command_recorder &recorder);

public:
	text_manager(const ste_context &context,
				 const font &default_font,
				 int default_size = 28);

	/**
	 *	@brief	Attaches a framebuffer. Affects all fragments created with this manager.
	 *			Expects a framebuffer with initialized color attachment at location 0 with initial layout gl::image_layout::color_attachment_optimal. Final layout remains
	 *			gl::image_layout::color_attachment_optimal.
	 */
	void set_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}

	text_fragment create_fragment();
};

}
}
