
#include "stdafx.hpp"
#include "deferred_composer.hpp"
#include "GIRenderer.hpp"

#include "Sampler.hpp"
#include "ShaderStorageBuffer.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;

deferred_composer::deferred_composer(const StEngineControl &ctx, GIRenderer *dr) : program(ctx.glslprograms_pool().fetch_program_task({ "passthrough.vert", "deferred_compose.frag" })()),
																				   dr(dr) {}

void deferred_composer::set_context_state() const {
	using namespace Core;

	auto &ls = dr->scene->scene_properties().lights_storage();

	GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);

	dr->gbuffer.bind_gbuffer();
	0_storage_idx = dr->scene->scene_properties().materials_storage().buffer();

	ls.bind_lights_buffer(2);

	dr->lll_storage.bind_lll_buffer();

	8_tex_unit = *dr->shadows_storage.get_cubemaps();
	8_sampler_idx = dr->shadows_storage.get_shadow_sampler();

	9_tex_unit = *dr->volumetric_scattering.get_volume_texture();
	9_sampler_idx = dr->volumetric_scattering.get_volume_sampler();

	ScreenFillingQuad.vao()->bind();

	program->bind();
}

void deferred_composer::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
