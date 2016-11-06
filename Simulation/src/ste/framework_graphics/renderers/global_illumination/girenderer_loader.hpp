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
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->hdr.wait();
			object->fxaa.wait();
			object->downsample_depth.wait();
			object->prepopulate_depth_dispatch.wait();
			object->prepopulate_backface_depth_dispatch.wait();
			object->scene_geo_cull.wait();
			object->composer.wait();
			object->shadows_projector.wait();
			object->directional_shadows_projector.wait();
			object->lll_gen_dispatch.wait();
			object->light_preprocess.wait();
			object->vol_scat_gather.wait();
			object->vol_scat_scatter.wait();
		}).then_on_main_thread([object]() {
			object->init();
		});
	}
};

}
}
