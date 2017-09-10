// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <fragment_compute.hpp>

#include <scene.hpp>
#include <light_storage.hpp>

namespace ste {
namespace graphics {

class scene_geo_cull_fragment : public gl::fragment_compute<scene_geo_cull_fragment> {
	using Base = gl::fragment_compute<scene_geo_cull_fragment>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	scene *s;
	const light_storage *ls;
	std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs(gl::command_recorder &recorder) {
		auto size = s->get_object_group().get_draw_buffers().draw_count();
		if (size != old_object_group_size) {
			old_object_group_size = size;
			s->resize_indirect_command_buffers(recorder, size);
		}
	}

public:
	scene_geo_cull_fragment(const gl::rendering_system &rs,
							scene *s,
							const light_storage *ls)
		: Base(rs,
			   "scene_geo_cull.comp"),
		s(s),
		ls(ls)
	{
		pipeline()["counter_data"] = gl::bind(s->get_culled_objects_counter());
		pipeline()["idb_data"] = gl::bind(s->get_idb().get());
		pipeline()["sidb_data"] = gl::bind(s->get_shadow_projection_buffers().idb.get());
		pipeline()["dsidb_data"] = gl::bind(s->get_directional_shadow_projection_buffers().idb.get());

		pipeline()["drawid_to_lightid_ttl_data"] = gl::bind(s->get_shadow_projection_buffers().proj_id_to_light_id_translation_table);
		pipeline()["d_drawid_to_lightid_ttl_data"] = gl::bind(s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table);

		dispatch_task.attach_pipeline(pipeline());
	}
	~scene_geo_cull_fragment() noexcept {}

	scene_geo_cull_fragment(scene_geo_cull_fragment&&) = default;

	static lib::string name() { return "geo_cull"; }

	void record(gl::command_recorder &recorder) override final {
		commit_idbs(recorder);

		auto& draw_buffers = s->get_object_group().get_draw_buffers();

		constexpr int jobs = 128;
		auto size = (draw_buffers.draw_count() + jobs - 1) / jobs;

		recorder << dispatch_task(static_cast<std::uint32_t>(size), 1u, 1u);
	}
};

}
}
