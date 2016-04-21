
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
	p->scene->scene_properties().lights_storage().bind_buffers(2);

	p->deferred->bind_output_textures();
	7_tex_unit = *p->deferred->z_buffer();
	7_sampler_idx = *Sampler::SamplerLinearClamp();
	8_tex_unit = *p->scene->shadow_storage().get_cubemaps();
	8_sampler_idx = *Sampler::SamplerAnisotropicLinearClamp();

	ssss_gen_program->bind();
}

void ssss_write_penumbras::dispatch() const {
	auto size = p->ssss->layers_size();

	Core::GL::gl_current_context::get()->dispatch_compute(size.x / 32, size.y / 32, 1);
}
