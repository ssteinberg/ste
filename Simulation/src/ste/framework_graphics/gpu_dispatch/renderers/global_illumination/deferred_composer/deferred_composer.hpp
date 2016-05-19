// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "gpu_dispatchable.hpp"

#include "GLSLProgram.hpp"
#include "Texture2D.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	std::shared_ptr<Core::GLSLProgram> program;
	GIRenderer *dr;

public:
	deferred_composer(const StEngineControl &ctx, GIRenderer *dr);
	~deferred_composer() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
