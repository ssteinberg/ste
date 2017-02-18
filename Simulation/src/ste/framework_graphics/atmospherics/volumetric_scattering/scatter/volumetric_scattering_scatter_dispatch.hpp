// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>
#include <gl_current_context.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>

#include <glsl_program.hpp>
#include <gpu_dispatchable.hpp>

#include <volumetric_scattering_storage.hpp>
#include <linked_light_lists.hpp>
#include <light_storage.hpp>
#include <shadowmap_storage.hpp>

#include <sampler.hpp>

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
	void attach_handles() const {
		auto depth_map = vss->get_depth_map();
		if (depth_map) {
			auto depth_map_handle = depth_map->get_texture_handle(*Core::sampler::sampler_linear_clamp());
			depth_map_handle.make_resident();
			program.get().set_uniform("depth_map", depth_map_handle);
		}

		auto downsampled_depth_map = vss->get_downsampled_depth_map();
		if (depth_map) {
			auto downsampled_depth_map_handle = downsampled_depth_map->get_texture_handle(vss->get_depth_sampler());
			downsampled_depth_map_handle.make_resident();
			program.get().set_uniform("downsampled_depth_map", downsampled_depth_map_handle);
		}

		auto shadow_map = shadows_storage->get_cubemaps();
		if (shadow_map) {
			auto shadow_map_handle = shadow_map->get_texture_handle(shadows_storage->get_shadow_sampler());
			shadow_map_handle.make_resident();
			program.get().set_uniform("shadow_depth_maps", shadow_map_handle);
		}

		auto directional_shadow_depth_maps = shadows_storage->get_directional_maps();
		if (directional_shadow_depth_maps) {
			auto directional_shadow_depth_maps_handle = directional_shadow_depth_maps->get_texture_handle(shadows_storage->get_shadow_sampler());
			directional_shadow_depth_maps_handle.make_resident();
			program.get().set_uniform("directional_shadow_depth_maps", directional_shadow_depth_maps_handle);
		}

		program.get().set_uniform("cascades_depths", ls->get_cascade_depths_array());
	}

private:
	volumetric_scattering_scatter_dispatch(const ste_engine_control &ctx,
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

	auto *get_program() { return &(program.get()); }
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::volumetric_scattering_scatter_dispatch> {
	using R = Graphics::volumetric_scattering_scatter_dispatch;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object]() {
			object->attach_handles();
		});
	}
};

}
}
