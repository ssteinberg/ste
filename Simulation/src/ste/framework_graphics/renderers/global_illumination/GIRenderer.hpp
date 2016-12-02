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

#include "atmospherics_properties.hpp"
#include "atmospherics_buffer.hpp"

#include "gpu_dispatchable.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"

#include "Scene.hpp"
#include "scene_prepopulate_depth_dispatch.hpp"
#include "scene_geo_cull_dispatch.hpp"

#include "light.hpp"
#include "light_preprocessor.hpp"
#include "linked_light_lists.hpp"
#include "linked_light_lists_gen_dispatch.hpp"

#include "hdr_dof_postprocess.hpp"

#include "fxaa_dispatchable.hpp"

#include "shadowmap_storage.hpp"
#include "shadowmap_projector.hpp"
#include "directional_shadowmap_projector.hpp"

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
	friend class Resource::resource_instance<GIRenderer>;

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
	using FbClearTask = StE::Graphics::fb_clear_dispatch<false>;

	static constexpr int view_transform_buffer_bind_location = 20;
	static constexpr int proj_transform_buffer_bind_location = 21;
	static constexpr int atmospherics_buffer_bind_location = 22;

private:
	const StEngineControl &ctx;

	deferred_gbuffer gbuffer;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	const Camera *camera;
	transforms_ring_buffers transform_buffers;
	atmospherics_buffer atmospheric_buffer;
	Scene *scene;

private:
	gpu_task::TaskCollection gui_tasks;
	gpu_task::TaskCollection added_tasks;
	
	FbClearTask fb_clearer;
	FbClearTask backface_fb_clearer;

	linked_light_lists lll_storage;
	shadowmap_storage shadows_storage;
	volumetric_scattering_storage vol_scat_storage;

	Resource::resource_instance<deferred_composer> composer;
	Resource::resource_instance<fxaa_dispatchable> fxaa;
	Resource::resource_instance<hdr_dof_postprocess> hdr;

	Resource::resource_instance<gbuffer_downsample_depth_dispatch> downsample_depth;
	Resource::resource_instance<scene_prepopulate_depth_dispatch> prepopulate_depth_dispatch;
	Resource::resource_instance<scene_prepopulate_depth_dispatch> prepopulate_backface_depth_dispatch;
	Resource::resource_instance<scene_geo_cull_dispatch> scene_geo_cull;

	Resource::resource_instance<linked_light_lists_gen_dispatch> lll_gen_dispatch;
	Resource::resource_instance<light_preprocessor> light_preprocess;

	Resource::resource_instance<shadowmap_projector> shadows_projector;
	Resource::resource_instance<directional_shadowmap_projector> directional_shadows_projector;	

	Resource::resource_instance<volumetric_scattering_scatter_dispatch> vol_scat_scatter;
	Resource::resource_instance<volumetric_scattering_gather_dispatch> vol_scat_gather;

	std::shared_ptr<const gpu_task> scene_task,
									composer_task,
									fxaa_task,
									fb_clearer_task,
									backface_fb_clearer_task,
									shadow_projector_task,
									directional_shadows_projector_task,
									volumetric_scattering_scatter_task,
									volumetric_scattering_gather_task,
									downsample_depth_task,
									prepopulate_depth_task,
									prepopulate_backface_depth_task,
									scene_geo_cull_task,
									lll_gen_task;

protected:
	void init();
	void setup_tasks();
	void rebuild_task_queue();

	static int gbuffer_depth_target_levels();

private:
	GIRenderer(const StEngineControl &ctx,
			   const Camera *camera,
			   Scene *scene,
			   const atmospherics_properties<float> &atmospherics_prop);

public:
	virtual ~GIRenderer() noexcept {}

	void update_atmospherics_properties(const atmospherics_properties<float> &atmospherics_prop) { atmospheric_buffer.update_data(atmospherics_prop); }

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
