
#include "stdafx.hpp"
#include "deferred_composer.hpp"
#include "GIRenderer.hpp"

#include "microfacet_refraction_ratio_fit.hpp"

#include "Quad.hpp"

#include "Sampler.hpp"

#include "gl_current_context.hpp"

#include "Log.hpp"
#include "AttributedString.hpp"
#include "attrib.hpp"

#include <fstream>

using namespace StE::Graphics;

deferred_composer::deferred_composer(const StEngineControl &ctx, GIRenderer *dr) : program(ctx, std::vector<std::string>{ "passthrough.vert", "deferred_compose.frag" }), dr(dr) {
	vss_storage_connection = std::make_shared<connection<>>([&]() {
		attach_handles();
	});
	shadows_storage_connection = std::make_shared<connection<>>([&]() {
		attach_handles();
	});
	dr->vol_scat_storage.get_storage_modified_signal().connect(vss_storage_connection);
	dr->shadows_storage.get_storage_modified_signal().connect(shadows_storage_connection);
}

void deferred_composer::load_microfacet_fit_lut() {
	using namespace Text::Attributes;

	std::ifstream ifs(R"(Data/microfacet_ggx_refraction_ratio_fit.bin)", std::ios::binary);
	if (!ifs.good()) {
		ste_log_error() << Text::AttributedString("Can't open ") + i("\"Data/microfacet_ggx_refraction_ratio_fit.bin\": ") + std::strerror(errno) << std::endl;
		throw std::runtime_error("\"Data/microfacet_ggx_refraction_ratio_fit.bin\" not found");
	}

	auto microfacet_refraction_ratio_fit_data = microfacet_refraction_ratio_fit(ifs).create_lut();
	ifs.close();

	microfacet_refraction_ratio_fit_lut = std::make_unique<Core::Texture2DArray>(microfacet_refraction_ratio_fit_data);

	auto handle = microfacet_refraction_ratio_fit_lut->get_texture_handle(*Core::Sampler::SamplerNearestClamp());
	handle.make_resident();
	program.get().set_uniform("microfacet_refraction_ratio_fit_lut", handle);

	ste_log() << Text::AttributedString("Loaded \"") + i("microfacet_ggx_refraction_ratio_fit.bin") + "\" successfully." << std::endl;
}

void deferred_composer::attach_handles() const {
	auto scattering_volume = dr->vol_scat_storage.get_volume_texture();
	if (scattering_volume) {
		auto scattering_volume_handle = scattering_volume->get_texture_handle(dr->vol_scat_storage.get_volume_sampler());
		scattering_volume_handle.make_resident();
		program.get().set_uniform("scattering_volume", scattering_volume_handle);
	}

	auto shadow_depth_maps = dr->shadows_storage.get_cubemaps();
	if (shadow_depth_maps) {
		auto shadow_depth_maps_handle = shadow_depth_maps->get_texture_handle(dr->shadows_storage.get_shadow_sampler());
		shadow_depth_maps_handle.make_resident();
		program.get().set_uniform("shadow_depth_maps", shadow_depth_maps_handle);

		auto shadow_maps_handle = shadow_depth_maps->get_texture_handle(*Core::Sampler::SamplerLinearClamp());
		shadow_maps_handle.make_resident();
		program.get().set_uniform("shadow_maps", shadow_maps_handle);
	}
}

void deferred_composer::set_context_state() const {
	using namespace Core;

	auto &ls = dr->scene->scene_properties().lights_storage();

	GL::gl_current_context::get()->enable_state(StE::Core::GL::BasicStateName::TEXTURE_CUBE_MAP_SEAMLESS);
	
	0_tex_unit = *dr->gbuffer.get_backface_depth_target();
	1_tex_unit = *dr->gbuffer.get_depth_target();

	dr->gbuffer.bind_gbuffer();
	0_storage_idx = dr->scene->scene_properties().materials_storage().buffer();
	1_storage_idx = dr->scene->scene_properties().material_layers_storage().buffer();

	ls.bind_lights_buffer(2);

	dr->lll_storage.bind_lll_buffer();

	ScreenFillingQuad.vao()->bind();

	program.get().bind();
}

void deferred_composer::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
