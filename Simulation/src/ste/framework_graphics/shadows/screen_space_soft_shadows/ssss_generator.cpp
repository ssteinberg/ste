
#include "stdafx.hpp"
#include "ssss_generator.hpp"

using namespace StE::Graphics;

void ssss_generator::set_context_state() const {
	using namespace Core;

	Core::gl_current_context::get()->enable_state(StE::Core::context_state_name::TEXTURE_CUBE_MAP_SEAMLESS);

	0_image_idx = ssss->get_penumbra_layers()->make_image(0);
	scene->scene_properties().lights_storage().bind_buffers(2);

	deferred->bind_output_textures();
	7_tex_unit = *scene->shadow_storage().get_cubemaps();

	ssss_gen_program->bind();
}

void ssss_generator::dispatch() const {
	auto size = ssss->layers_size();

	Core::gl_current_context::get()->dispatch_compute(size.x / 32, size.y / 32, 1);
}
