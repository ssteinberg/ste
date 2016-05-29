// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "glsl_program.hpp"
#include "gpu_dispatchable.hpp"

#include "volumetric_scattering_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_gather_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<volumetric_scattering_gather_dispatch>;

private:
	const volumetric_scattering_storage *vss;

	Resource::resource_instance<Core::glsl_program> program;

public:
	volumetric_scattering_gather_dispatch(const StEngineControl &ctx, const volumetric_scattering_storage *vss) : vss(vss) {
		program.load(ctx, "volumetric_scattering_gather.glsl");
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::volumetric_scattering_gather_dispatch> {
	using R = Graphics::volumetric_scattering_gather_dispatch;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, args...);

			object->program.wait();

			return object;
		});
	}
};

}
}
