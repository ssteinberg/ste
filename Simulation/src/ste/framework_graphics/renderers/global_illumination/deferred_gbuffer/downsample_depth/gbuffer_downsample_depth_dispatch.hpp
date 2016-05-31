// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "Sampler.hpp"

#include "glsl_program.hpp"
#include "deferred_gbuffer.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class gbuffer_downsample_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<gbuffer_downsample_depth_dispatch>;
	friend class Resource::resource_instance<gbuffer_downsample_depth_dispatch>;

private:
	const deferred_gbuffer *gbuffer;
	Resource::resource_instance<Resource::glsl_program> program;

	std::shared_ptr<connection<>> gbuffer_depth_target_connection;

private:
	void attach_handles() const {
		auto depth_target = gbuffer->get_depth_target();
		if (depth_target) {
			auto depth_target_handle = depth_target->get_texture_handle(*Core::Sampler::SamplerNearestClamp());
			depth_target_handle.make_resident();
			program.get().set_uniform("depth_target", depth_target_handle);
		}

		auto downsampled_depth_target = gbuffer->get_downsampled_depth_target();
		if (downsampled_depth_target) {
			auto downsampled_depth_target_handle = downsampled_depth_target->get_texture_handle(*Core::SamplerMipmapped::MipmappedSamplerNearestClamp());
			downsampled_depth_target_handle.make_resident();
			program.get().set_uniform("downsampled_depth_target", downsampled_depth_target_handle);
		}
	}

public:
	gbuffer_downsample_depth_dispatch(const StEngineControl &ctx,
									  const deferred_gbuffer *gbuffer) : gbuffer(gbuffer),
									  									 program(ctx, "gbuffer_downsample_depth.glsl") {
		gbuffer_depth_target_connection = std::make_shared<connection<>>([&]() {
			attach_handles();
		});
		gbuffer->get_depth_target_modified_signal().connect(gbuffer_depth_target_connection);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::gbuffer_downsample_depth_dispatch> {
	using R = Graphics::gbuffer_downsample_depth_dispatch;

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
