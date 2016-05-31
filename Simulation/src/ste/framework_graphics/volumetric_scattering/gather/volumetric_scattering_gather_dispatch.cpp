
#include "stdafx.hpp"
#include "volumetric_scattering_gather_dispatch.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void volumetric_scattering_gather_dispatch::set_context_state() const {
	7_image_idx = vss->get_volume_texture()->make_image();

	11_tex_unit = *vss->get_depth_map();
	11_sampler_idx = vss->get_depth_sampler();

	program.get().bind();
}

void volumetric_scattering_gather_dispatch::dispatch() const {
	const glm::ivec2 jobs = { 32, 32 };
	auto size = (vss->get_size().xy() + jobs - glm::ivec2(1)) / jobs;

	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
}
