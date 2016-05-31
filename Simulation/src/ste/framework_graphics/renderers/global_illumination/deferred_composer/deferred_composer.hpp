// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "gpu_dispatchable.hpp"

#include "glsl_program.hpp"
#include "Texture2D.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	Resource::resource_instance<Resource::glsl_program> program;
	GIRenderer *dr;

public:
	deferred_composer(const StEngineControl &ctx, GIRenderer *dr) : program(ctx, std::vector<std::string>{ "passthrough.vert", "deferred_compose.frag" }), dr(dr) {}
	~deferred_composer() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
