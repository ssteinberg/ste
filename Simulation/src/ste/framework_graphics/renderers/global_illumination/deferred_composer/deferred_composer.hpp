// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "glsl_program.hpp"
#include "Texture2D.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<deferred_composer>;

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

namespace Resource {

template <>
class resource_loading_task<Graphics::deferred_composer> {
	using R = Graphics::deferred_composer;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		});
	}
};

}
}
