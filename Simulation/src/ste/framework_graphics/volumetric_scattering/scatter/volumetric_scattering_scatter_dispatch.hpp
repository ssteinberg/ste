// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

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
	friend class Resource::resource_instance<volumetric_scattering_scatter_dispatch>;

private:
	const volumetric_scattering_storage *vss;
	const linked_light_lists *llls;
	const light_storage *ls;
	const shadowmap_storage *shadows_storage;

	Resource::resource_instance<Resource::glsl_program> program;

	std::shared_ptr<connection<>> vss_storage_connection;
	std::shared_ptr<connection<>> shadows_storage_connection;

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

	void attach_handles() const {
		auto depth_map = vss->get_depth_map();
		if (depth_map) {
			auto depth_map_handle = depth_map->get_texture_handle(vss->get_depth_sampler());
			depth_map_handle.make_resident();
			program.get().set_uniform("depth_map", depth_map_handle);
		}

		auto shadow_map = shadows_storage->get_cubemaps();
		if (shadow_map) {
			auto shadow_map_handle = shadow_map->get_texture_handle(shadows_storage->get_shadow_sampler());
			shadow_map_handle.make_resident();
			program.get().set_uniform("shadow_depth_maps", shadow_map_handle);
		}
	}

private:
	volumetric_scattering_scatter_dispatch(const StEngineControl &ctx,
										   const volumetric_scattering_storage *vss,
										   const linked_light_lists *llls,
										   const light_storage *ls,
										   const shadowmap_storage *shadows_storage) : vss(vss),
										   											   llls(llls),
																					   ls(ls),
																					   shadows_storage(shadows_storage),
																					   program(ctx, "volumetric_scattering_scatter.glsl") {
		vss_storage_connection = std::make_shared<connection<>>([&]() {
			attach_handles();
		});
		shadows_storage_connection = std::make_shared<connection<>>([&]() {
			attach_handles();
		});
		vss->get_storage_modified_signal().connect(vss_storage_connection);
		shadows_storage->get_storage_modified_signal().connect(shadows_storage_connection);
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
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		}).then_on_main_thread([object]() {
			object->update_phase_uniforms(object->vss->get_scattering_phase_anisotropy_coefficient());
			object->attach_handles();
		});
	}
};

}
}
