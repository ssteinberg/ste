// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "GLSLProgramFactory.hpp"
#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "volumetric_scattering_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_gather_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const volumetric_scattering_storage *vss;

	std::shared_ptr<Core::GLSLProgram> program;

public:
	volumetric_scattering_gather_dispatch(const StEngineControl &ctx,
										  const volumetric_scattering_storage *vss) : vss(vss),
																					  program(Resource::GLSLProgramFactory::load_program_task(ctx, { "volumetric_scattering_gather.glsl" })()) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
