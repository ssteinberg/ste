// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "GIRenderer.hpp"

namespace StE {
namespace Resource {

template <>
class resource_loading_task<Graphics::GIRenderer> {
	using R = Graphics::GIRenderer;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			auto g0 =  object->lll_storage.load_and_wait_guard(ctx, ctx.get_backbuffer_size());
			auto g1 =  object->shadows_storage.load_and_wait_guard(ctx);
			auto g3 =  object->hdr.load_and_wait_guard(ctx, &object->gbuffer);
			auto g4 =  object->downsample_depth.load_and_wait_guard(ctx, &object->gbuffer);
			auto g5 =  object->prepopulate_depth_dispatch.load_and_wait_guard(ctx, object->scene);
			auto g6 =  object->scene_geo_cull.load_and_wait_guard(ctx, object->scene, &object->scene->scene_properties().lights_storage());
			auto g7 =  object->composer.load_and_wait_guard(ctx, object.get());

			auto g8 =  object->shadows_projector.load_and_wait_guard(ctx, object->scene, &object->scene->scene_properties().lights_storage(), &object->shadows_storage.get());
			auto g10 = object->lll_gen_dispatch.load_and_wait_guard(ctx, &object->scene->scene_properties().lights_storage(), &object->lll_storage.get());
			auto g11 = object->light_preprocess.load_and_wait_guard(ctx, &object->scene->scene_properties().lights_storage(), &object->hdr.get());

			auto g9 =  object->volumetric_scattering_gather.load_and_wait_guard(ctx, &object->volumetric_scattering);
			auto g12 = object->volumetric_scattering_scatter.load_and_wait_guard(ctx, &object->volumetric_scattering,
																				 &object->lll_storage.get(), &object->scene->scene_properties().lights_storage(),
																				 &object->shadows_storage.get());

			object->lll_gen_dispatch.get().set_depth_map(object->gbuffer.get_downsampled_depth_target());

			object->setup_tasks();
			object->rebuild_task_queue();

			return object;
		});
	}
};

}
}
