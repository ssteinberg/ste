// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"
#include "gl_current_context.hpp"

#include "ssss_generator.hpp"

#include "GLSLProgram.hpp"

#include "Quad.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_bilateral_blur_y : public gpu_dispatchable {
private:
	const ssss_generator *p;
	std::shared_ptr<Core::GLSLProgram> ssss_blur_program;

public:
	ssss_bilateral_blur_y(const ssss_generator *p, const StEngineControl &ctx) : p(p),
																				 ssss_blur_program(ctx.glslprograms_pool().fetch_program_task({ "ssss_blur.vert", "ssss_blur.geom", "ssss_blur_y.frag" })()) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
