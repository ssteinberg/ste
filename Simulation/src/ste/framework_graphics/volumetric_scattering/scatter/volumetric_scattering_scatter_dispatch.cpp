
#include "stdafx.hpp"
#include "volumetric_scattering_scatter_dispatch.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void volumetric_scattering_scatter_dispatch::set_context_state() const {
	GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);

	ls->bind_lights_buffer(2);

	llls->bind_lll_buffer();
	7_storage_idx = ls->get_directional_lights_cascades_buffer();

	7_image_idx = vss->get_volume_texture()->make_image().with_access(Core::ImageAccessMode::Write);

	program.get().bind();
}

void volumetric_scattering_scatter_dispatch::dispatch() const {
	static const glm::ivec2 jobs = { 32, 32 };
	auto size = (glm::ivec2{ vss->get_size().x, vss->get_size().y } + jobs - glm::ivec2(1)) / jobs;

	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
}
