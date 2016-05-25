// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "GLSLProgramFactory.hpp"

#include "deferred_gbuffer.hpp"
#include "GLSLProgram.hpp"

namespace StE {
namespace Graphics {

class gbuffer_sort_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	deferred_gbuffer *gbuffer;
	std::shared_ptr<Core::GLSLProgram> sort_program;

public:
	gbuffer_sort_dispatch(const StEngineControl &ctx, deferred_gbuffer *gbuffer) : gbuffer(gbuffer),
																				   sort_program(Resource::GLSLProgramFactory::load_program_task({ "gbuffer_sort.glsl" })()) {}

protected:
	virtual void set_context_state() const override {
		gbuffer->bind_gbuffer();
		sort_program->bind();
	}

	virtual void dispatch() const override {
		constexpr int jobs = 32;
		auto size = (gbuffer->get_size() + glm::ivec2(jobs - 1)) / jobs;

		Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
	}
};

}
}
