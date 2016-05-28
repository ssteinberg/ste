// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "glsl_program.hpp"
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

	friend class Resource::resource_loading_task<volumetric_scattering_scatter_dispatch>;

private:
	const volumetric_scattering_storage *vss;
	const linked_light_lists *llls;
	const light_storage *ls;
	const shadowmap_storage *shadows_storage;

	Resource::resource_instance<Core::glsl_program> program;

private:
	void update_phase_uniforms(float g) {
		float g2 = g * g;
		float p1 = (1.f - g2) / (4.f * glm::pi<float>());
		float p2 = 1.f + g2;
		float p3 = 2.f * g;

		program.get().set_uniform("phase1", p1);
		program.get().set_uniform("phase2", p2);
		program.get().set_uniform("phase3", p3);
	}

	volumetric_scattering_scatter_dispatch(const StEngineControl &ctx,
										   const volumetric_scattering_storage *vss,
										   const linked_light_lists *llls,
										   const light_storage *ls,
										   const shadowmap_storage *shadows_storage) : vss(vss), llls(llls), ls(ls), shadows_storage(shadows_storage) {
		program.load(ctx, "volumetric_scattering_scatter.glsl");
	}

public:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::volumetric_scattering_scatter_dispatch> {
	using R = Graphics::volumetric_scattering_scatter_dispatch;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->program.wait();
			object->update_phase_uniforms(object->vss->get_scattering_phase_anisotropy_coefficient());

			return object;
		});
	}
};

}
}
