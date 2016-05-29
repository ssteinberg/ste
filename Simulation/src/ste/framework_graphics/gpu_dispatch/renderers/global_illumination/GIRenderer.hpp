// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "rendering_system.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "Camera.hpp"
#include "transforms_ring_buffers.hpp"

#include "gpu_dispatchable.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"

#include "Scene.hpp"
#include "scene_prepopulate_depth_dispatch.hpp"
#include "scene_geo_cull_dispatch.hpp"

#include "SceneProperties.hpp"

#include "light.hpp"
#include "light_preprocessor.hpp"
#include "linked_light_lists.hpp"
#include "linked_light_lists_gen_dispatch.hpp"

#include "gpu_dummy_dispatchable.hpp"
#include "hdr_dof_postprocess.hpp"

#include "shadowmap_storage.hpp"
#include "shadowmap_projector.hpp"

#include "volumetric_scattering_storage.hpp"
#include "volumetric_scattering_scatter_dispatch.hpp"
#include "volumetric_scattering_gather_dispatch.hpp"

#include "deferred_composer.hpp"

#include "deferred_gbuffer.hpp"
#include "gbuffer_downsample_depth_dispatch.hpp"

#include "dense_voxel_space.hpp"
#include "fb_clear_dispatch.hpp"

#include "glsl_program.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer : public rendering_system {
	using Base = rendering_system;

	friend class deferred_composer;
	friend class Resource::resource_loading_task<GIRenderer>;

	struct ctor_token {};

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
	using FbClearTask = StE::Graphics::fb_clear_dispatch<false>;

	constexpr static int view_transform_buffer_bind_location = 20;
	constexpr static int proj_transform_buffer_bind_location = 21;

private:
	const StEngineControl &ctx;

	deferred_gbuffer gbuffer;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	const Camera *camera;
	transforms_ring_buffers transform_buffers;
	Scene *scene;

private:
	gpu_task::TaskCollection gui_tasks;
	gpu_task::TaskCollection added_tasks;

	FbClearTask fb_clearer;

	linked_light_lists lll_storage;
	shadowmap_storage shadows_storage;
	volumetric_scattering_storage vol_scat_storage;

	Resource::resource_instance<deferred_composer> composer;

	Resource::resource_instance<linked_light_lists_gen_dispatch> lll_gen_dispatch;
	Resource::resource_instance<light_preprocessor> light_preprocess;

	Resource::resource_instance<shadowmap_projector> shadows_projector;

	Resource::resource_instance<volumetric_scattering_scatter_dispatch> vol_scat_scatter;
	Resource::resource_instance<volumetric_scattering_gather_dispatch> vol_scat_gather;

	Resource::resource_instance<hdr_dof_postprocess> hdr;
	Resource::resource_instance<gbuffer_downsample_depth_dispatch> downsample_depth;

	Resource::resource_instance<scene_prepopulate_depth_dispatch> prepopulate_depth_dispatch;
	Resource::resource_instance<scene_geo_cull_dispatch> scene_geo_cull;

	std::shared_ptr<const gpu_task> precomposer_dummy_task,
									scene_task,
									composer_task,
									fb_clearer_task,
									shadow_projector_task,
									volumetric_scattering_scatter_task,
									volumetric_scattering_gather_task,
									downsample_depth_task,
									prepopulate_depth_task,
									scene_geo_cull_task,
									lll_gen_task;

	bool use_deferred_rendering{ true };

protected:
	void setup_tasks();
	void rebuild_task_queue();

	static int gbuffer_depth_target_levels();

	const Core::GenericFramebufferObject *get_fbo() const {
		if (use_deferred_rendering)
			return gbuffer.get_fbo();
		return &ctx.gl()->defaut_framebuffer();
	}

public:
	GIRenderer(ctor_token,
			   const StEngineControl &ctx,
			   const Camera *camera,
			   Scene *scene);
	virtual ~GIRenderer() noexcept {}

	void set_deferred_rendering_enabled(bool enabled);

	void add_task(const gpu_task::TaskPtr &t);
	void remove_task(const gpu_task::TaskPtr &t);
	void add_gui_task(const gpu_task::TaskPtr &t);
	void remove_gui_task(const gpu_task::TaskPtr &t);

	virtual void render_queue() override;

	auto *get_gbuffer() const { return &gbuffer; }
	auto &get_scene_task() const { return scene_task; }

	void attach_profiler(profiler *p) { q.attach_profiler(p); }

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}

#include "girenderer_loader.hpp"
