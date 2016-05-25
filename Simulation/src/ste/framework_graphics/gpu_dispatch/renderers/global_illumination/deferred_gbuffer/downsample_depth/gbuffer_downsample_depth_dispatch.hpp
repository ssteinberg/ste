// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgramFactory.hpp"

#include "GLSLProgram.hpp"
#include "deferred_gbuffer.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class gbuffer_downsample_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const deferred_gbuffer *gbuffer;

	std::shared_ptr<Core::GLSLProgram> program;

public:
	gbuffer_downsample_depth_dispatch(const StEngineControl &ctx,
									  const deferred_gbuffer *gbuffer) : gbuffer(gbuffer),
																		 program(Resource::GLSLProgramFactory::load_program_task(ctx, { "gbuffer_downsample_depth.glsl" })()) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
