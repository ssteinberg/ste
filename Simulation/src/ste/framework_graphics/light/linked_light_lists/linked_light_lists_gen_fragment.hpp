// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

namespace ste {
namespace graphics {

class linked_light_lists_gen_fragment : public gl::fragment_compute<linked_light_lists_gen_fragment> {
	using Base = gl::fragment_compute<linked_light_lists_gen_fragment>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	linked_light_lists *lll;

public:
	linked_light_lists_gen_fragment(const gl::rendering_system &rs,
									linked_light_lists *lll)
		: Base(rs,
			   "linked_light_lists_gen.comp"),
		lll(lll)
	{
		dispatch_task.attach_pipeline(pipeline());
	}
	~linked_light_lists_gen_fragment() noexcept {}

	linked_light_lists_gen_fragment(linked_light_lists_gen_fragment&&) = default;

	static lib::string name() { return "lll"; }

	void record(gl::command_recorder &recorder) override final {
		static const glm::ivec2 jobs = { 32, 32 };

		const auto size = (glm::ivec2{ lll->get_extent().x, lll->get_extent().y } + jobs - glm::ivec2(1)) / jobs;

		lll->clear(recorder);
		recorder << dispatch_task(static_cast<std::uint32_t>(size.x), 
								  static_cast<std::uint32_t>(size.y),
								  1u);
	}
};

}
}
