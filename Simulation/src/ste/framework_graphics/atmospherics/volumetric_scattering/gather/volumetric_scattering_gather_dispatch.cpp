
#include "stdafx.hpp"
#include "volumetric_scattering_gather_dispatch.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void volumetric_scattering_gather_dispatch::set_context_state() const {
	7_image_idx = vss->get_volume_texture()->make_image();

	program.get().bind();
}

void volumetric_scattering_gather_dispatch::dispatch() const {
	const glm::ivec2 jobs = { 32, 32 };
	auto vss_size = vss->get_size();
	auto size = (glm::ivec2{ vss_size.x, vss_size.y } + jobs - glm::ivec2(1)) / jobs;

	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
}
