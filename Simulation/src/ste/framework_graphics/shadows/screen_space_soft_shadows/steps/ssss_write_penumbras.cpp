
#include "stdafx.hpp"
#include "ssss_write_penumbras.hpp"

#include "gl_current_context.hpp"
#include "Sampler.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void ssss_write_penumbras::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);

	0_image_idx = p->ssss->get_penumbra_layers()->make_image(0);
	1_image_idx = p->ssss->get_z_buffer()->make_image();
	p->scene->scene_properties().lights_storage().bind_lights_buffer(2);

	p->gbuffer->bind_gbuffer();
	8_tex_unit = *p->shadows_storage->get_cubemaps();
	8_sampler_idx = *Sampler::SamplerAnisotropicLinearClamp();

	ssss_gen_program->bind();
}

void ssss_write_penumbras::dispatch() const {
	constexpr int jobs = 32;
	auto size = (p->ssss->layers_size() + glm::ivec2(jobs - 1)) / jobs;

	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
}
