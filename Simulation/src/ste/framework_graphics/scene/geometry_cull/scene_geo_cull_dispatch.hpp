// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <fragment_compute.hpp>

#include <scene.hpp>
#include <light_storage.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class scene_geo_cull_dispatch : public gl::fragment_compute {
	using Base = gl::fragment_compute;

	gl::task<gl::cmd_dispatch> dispatch_task;

	const scene *s;
	const light_storage *ls;
	std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs(gl::command_recorder &recorder) const {
		auto size = s->get_object_group().get_draw_buffers().size();
		if (size != old_object_group_size) {
			old_object_group_size = size;
			s->resize_indirect_command_buffers(recorder, size);
		}
	}

public:
	scene_geo_cull_dispatch(const gl::rendering_system &rs,
							const scene *s,
							const light_storage *ls)
		: Base(rs,
			   "scene_geo_cull.comp"),
		s(s),
		ls(ls)
	{
		dispatch_task.attach_pipeline(pipeline);
//		program.get().set_uniform("cascades_depths", s->properties().lights_storage().get_cascade_depths_array());
	}
	~scene_geo_cull_dispatch() noexcept {}

	static const lib::string& name() { return "geo_cull"; }

	void record(gl::command_recorder &recorder) override final {
		commit_idbs(recorder);

		auto& draw_buffers = s->get_object_group().get_draw_buffers();

		constexpr int jobs = 128;
		auto size = (draw_buffers.draw_count() + jobs - 1) / jobs;

		s->clear_indirect_command_buffers(recorder);
		recorder << dispatch_task(size, 1, 1);
	}
};

}
}
