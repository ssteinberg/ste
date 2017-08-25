// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <light_storage.hpp>


namespace ste {
namespace graphics {

class light_preprocessor_fragment : public gl::fragment_compute {
	using Base = fragment_compute;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	light_storage *ls;

private:
	void set_projection_planes() const;

public:
	light_preprocessor_fragment(const gl::rendering_system &rs,
								light_storage *ls)
		: Base(rs,
			   "light_preprocess_cull_lights.comp"),
		ls(ls)
	{
		set_projection_planes();
		dispatch_task.attach_pipeline(pipeline);
	}
	~light_preprocessor_fragment() noexcept {}

	light_preprocessor_fragment(light_preprocessor_fragment&&) = default;

	static const lib::string& name() { return "cull_lights"; }

	void record(gl::command_recorder &recorder) override final {
		constexpr int jobs = 128;
		auto size = (ls->size() + jobs - 1) / jobs;

		ls->clear_active_ll(recorder);
		recorder << dispatch_task(static_cast<std::uint32_t>(size), 1u, 1u);
	}
};

}
}
