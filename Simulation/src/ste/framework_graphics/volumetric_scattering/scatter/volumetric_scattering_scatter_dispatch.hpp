// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "volumetric_scattering_storage.hpp"
#include "linked_light_lists.hpp"
#include "light_storage.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_scatter_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const volumetric_scattering_storage *vss;
	const linked_light_lists *llls;
	const light_storage *ls;
	const shadowmap_storage *shadows_storage;

	std::shared_ptr<Core::GLSLProgram> program;

private:
	void update_phase_uniforms(float g) {
		float g2 = g * g;
		float p1 = (1.f - g2) / (4.f * glm::pi<float>());
		float p2 = 1.f + g2;
		float p3 = 2.f * g;

		program->set_uniform("phase1", p1);
		program->set_uniform("phase2", p2);
		program->set_uniform("phase3", p3);
	}

public:
	volumetric_scattering_scatter_dispatch(const StEngineControl &ctx,
										   const volumetric_scattering_storage *vss,
										   const linked_light_lists *llls,
										   const light_storage *ls,
										   const shadowmap_storage *shadows_storage) : vss(vss), llls(llls), ls(ls), shadows_storage(shadows_storage),
																					   program(ctx.glslprograms_pool().fetch_program_task({ "volumetric_scattering_scatter.glsl" })()) {
		update_phase_uniforms(vss->get_scattering_phase_anisotropy_coefficient());
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
