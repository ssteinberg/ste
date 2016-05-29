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
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now_on_main_thread([=, &ctx]() {
			return std::make_unique<R>(R::ctor_token(), ctx, args...);
		}).then([](std::unique_ptr<R> &&object) {
			object->hdr.wait();
			object->downsample_depth.wait();
			object->prepopulate_depth_dispatch.wait();
			object->scene_geo_cull.wait();
			object->composer.wait();
			object->shadows_projector.wait();
			object->lll_gen_dispatch.wait();
			object->light_preprocess.wait();
			object->vol_scat_gather.wait();
			object->vol_scat_scatter.wait();

			object->lll_gen_dispatch.get().set_depth_map(object->gbuffer.get_downsampled_depth_target());

			return std::move(object);
		}).then_on_main_thread([](std::unique_ptr<R> &&object) {
			object->setup_tasks();
			object->add_task(object->fb_clearer_task);
			object->rebuild_task_queue();

			return std::move(object);
		});
	}
};

}
}
