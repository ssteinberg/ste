// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "glsl_program_loading_task.hpp"
#include "Texture2D.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer;

class deferred_composer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	Resource::resource_instance<Core::glsl_program> program;
	GIRenderer *dr;

public:
	deferred_composer(const StEngineControl &ctx, GIRenderer *dr) : dr(dr) {
		program.load(ctx, std::vector<std::string>{ "passthrough.vert", "deferred_compose.frag" });
	}
	~deferred_composer() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<deferred_composer> {
	using R = deferred_composer;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->program.wait();

			return object;
		});
	}
};

}
}
