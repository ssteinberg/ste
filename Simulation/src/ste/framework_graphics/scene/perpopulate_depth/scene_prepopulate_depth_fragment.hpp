// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_graphics.hpp>

#include <cmd_draw_indexed_indirect.hpp>

#include <scene.hpp>
#include <object_vertex_data.hpp>

namespace ste {
namespace graphics {

template <bool front_face>
class scene_prepopulate_depth_fragment : public gl::fragment_graphics<scene_prepopulate_depth_fragment<front_face>> {
	using Base = gl::fragment_graphics<scene_prepopulate_depth_fragment<front_face>>;

private:
	gl::task<gl::cmd_draw_indexed_indirect> draw_task;

	const scene *s;

public:
	scene_prepopulate_depth_fragment(const gl::rendering_system &rs,
									 const scene *s)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "scene_transform.vert", "scene_prepopulate_depth.frag"),
		s(s)
	{
		draw_task.attach_pipeline(pipeline());
		draw_task.attach_vertex_buffer(s->get_object_group().get_draw_buffers().get_vertex_buffer());
		draw_task.attach_index_buffer(s->get_object_group().get_draw_buffers().get_index_buffer());
		draw_task.attach_indirect_buffer(s->get_idb());
	}
	~scene_prepopulate_depth_fragment() noexcept {}

	scene_prepopulate_depth_fragment(scene_prepopulate_depth_fragment&&) = default;

	static auto create_fb_layout() {
		gl::framebuffer_layout fb_layout;
		fb_layout[gl::pipeline_depth_attachment_location] = gl::clear_store(gl::format::d32_sfloat,
																			gl::image_layout::shader_read_only_optimal);
		return fb_layout;
	}

	static lib::string name() { return "prepopulate_depth"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		auditor.set_framebuffer_layout(create_fb_layout());

		gl::device_pipeline_graphics_configurations config;
		config.depth_op = gl::depth_operation(gl::compare_op::greater);
		config.rasterizer_op = gl::rasterizer_operation(front_face ?
														gl::cull_mode::back_bit :
														gl::cull_mode::front_bit,
														gl::front_face::ccw);
		auditor.set_pipeline_settings(std::move(config));
		auditor.set_vertex_attributes(0, gl::vertex_attributes<object_vertex_data>());
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline().attach_framebuffer(fb);
	}
	const auto& get_framebuffer_layout() const {
		return pipeline().get_framebuffer_layout();
	}

	void record(gl::command_recorder &recorder) override final {
		const auto draw_count = s->get_object_group().draw_count();

		recorder << draw_task(static_cast<std::uint32_t>(draw_count));
	}
};

using scene_prepopulate_depth_front_face_fragment = scene_prepopulate_depth_fragment<true>;
using scene_prepopulate_depth_back_face_fragment = scene_prepopulate_depth_fragment<false>;

}
}
