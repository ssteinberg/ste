// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "glsl_program.hpp"
#include "gpu_dispatchable.hpp"

#include "volumetric_scattering_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_gather_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<volumetric_scattering_gather_dispatch>;
	friend class Resource::resource_instance<volumetric_scattering_gather_dispatch>;

private:
	const volumetric_scattering_storage *vss;

	Resource::resource_instance<Resource::glsl_program> program;

	std::shared_ptr<connection<>> vss_storage_connection;

	void attach_handles() const {
		auto dm = vss->get_depth_map();
		if (dm != nullptr) {
			auto depth_map_handle = vss->get_depth_map()->get_texture_handle(vss->get_depth_sampler());
			depth_map_handle.make_resident();
			program.get().set_uniform("depth_map", depth_map_handle);
		}
	}

private:
	volumetric_scattering_gather_dispatch(const StEngineControl &ctx,
										  const volumetric_scattering_storage *vss) : vss(vss),
																					  program(ctx, "volumetric_scattering_gather.glsl") {
		vss_storage_connection = std::make_shared<connection<>>([&]() {
			attach_handles();
		});
		vss->get_storage_modified_signal().connect(vss_storage_connection);
	}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::volumetric_scattering_gather_dispatch> {
	using R = Graphics::volumetric_scattering_gather_dispatch;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		}).then_on_main_thread([object]() {
			object->attach_handles();
		});
	}
};

}
}
