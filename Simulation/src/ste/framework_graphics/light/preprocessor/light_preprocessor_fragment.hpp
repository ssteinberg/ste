// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <light_storage.hpp>
#include <primary_renderer_camera.hpp>


namespace ste {
namespace graphics {

class light_preprocessor_fragment : public gl::fragment_compute<light_preprocessor_fragment> {
	using Base = fragment_compute<light_preprocessor_fragment>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	light_storage *ls;

public:
	light_preprocessor_fragment(const gl::rendering_system &rs,
								light_storage *ls,
								const primary_renderer_camera &camera)
		: Base(rs,
			   "light_preprocess_cull_lights.comp"),
		ls(ls)
	{
		update_projection_planes(camera);
		dispatch_task.attach_pipeline(pipeline());
	}
	~light_preprocessor_fragment() noexcept {}

	light_preprocessor_fragment(light_preprocessor_fragment&&) = default;

	static lib::string name() { return "cull_lights"; }

	/**
	 *	@brief	Updates the fragment's projection planes uniform buffer. 
	 *			Should be called whenever camera's projection model changes.
	 */
	void update_projection_planes(const primary_renderer_camera &camera);

	void record(gl::command_recorder &recorder) override final {
		constexpr int jobs = 128;
		const auto size = (ls->size() + jobs - 1) / jobs;

		pipeline()["push_t.lights_length"] = static_cast<std::uint32_t>(ls->size());

		recorder << dispatch_task(static_cast<std::uint32_t>(size), 1u, 1u);
	}
};

}
}
