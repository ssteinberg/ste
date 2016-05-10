// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "rendering_system.hpp"

#include "Camera.hpp"
#include "view_matrix_ring_buffer.hpp"

#include "gpu_dispatchable.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"

#include "Scene.hpp"
#include "scene_prepopulate_depth_dispatch.hpp"
#include "scene_frustum_cull_dispatch.hpp"

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

#include "deferred_gbuffer.hpp"
#include "gbuffer_clear_dispatch.hpp"
#include "gbuffer_sort_dispatch.hpp"

#include "dense_voxel_space.hpp"
#include "fb_clear_dispatch.hpp"

#include "GLSLProgram.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer : public rendering_system {
	using Base = rendering_system;

private:
	class deferred_composition : public gpu_dispatchable {
		using Base = gpu_dispatchable;

		friend class GIRenderer;

	private:
		std::shared_ptr<Core::GLSLProgram> program;
		GIRenderer *dr;

	public:
		deferred_composition(const StEngineControl &ctx, GIRenderer *dr) : program(ctx.glslprograms_pool().fetch_program_task({ "passthrough.vert", "deferred_compose.frag" })()), dr(dr) {
			// dr->voxel_space.add_consumer_program(this->get_program());
		}
		~deferred_composition() {
			// dr->voxel_space.remove_consumer_program(this->get_program());
		}

	protected:
		void set_context_state() const override final;
		void dispatch() const override final;
	};

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
	using FbClearTask = StE::Graphics::fb_clear_dispatch<>;

	constexpr static int view_matrix_buffer_bind_location = 20;

private:
	deferred_gbuffer gbuffer;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	const StEngineControl &ctx;
	const Camera *camera;
	view_matrix_ring_buffer view_matrix_buffer;
	Scene *scene;
	// dense_voxel_space voxel_space;

private:
	gpu_task::TaskCollection gui_tasks;
	gpu_task::TaskCollection added_tasks;

	linked_light_lists lll_storage;
	linked_light_lists_gen_dispatch lll_gen_dispatch;
	light_preprocessor light_preprocess;

	shadowmap_storage shadows_storage;
	shadowmap_projector shadows_projector;

	volumetric_scattering_storage volumetric_scattering;
	volumetric_scattering_scatter_dispatch volumetric_scattering_scatter;
	volumetric_scattering_gather_dispatch volumetric_scattering_gather;

	hdr_dof_postprocess hdr;
	gbuffer_sort_dispatch gbuffer_sorter;

	scene_prepopulate_depth_dispatch prepopulate_depth_dispatch;
	scene_frustum_cull_dispatch scene_frustum_cull;

	std::shared_ptr<const gpu_task> precomposer_dummy_task,
									scene_task,
									composer_task,
									fb_clearer_task,
									gbuffer_clearer_task,
									shadow_projector_task,
									volumetric_scattering_scatter_task,
									volumetric_scattering_gather_task,
									gbuffer_sort_task,
									prepopulate_depth_task,
									scene_frustum_cull_task,
									lll_gen_task;

	deferred_composition composer;
	gbuffer_clear_dispatch gbuffer_clearer;
	FbClearTask fb_clearer;

	bool use_deferred_rendering{ true };

protected:
	void rebuild_task_queue();
	void update_shader_proj_uniforms(const glm::mat4&);

	const Core::GenericFramebufferObject *get_fbo() const {
		if (use_deferred_rendering)
			return gbuffer.get_fbo();
		return &ctx.gl()->defaut_framebuffer();
	}

public:
	GIRenderer(const StEngineControl &ctx,
			   const Camera *camera,
			   Scene *scene/*,
			   std::size_t voxel_grid_size = 512,
			   float voxel_grid_ratio = .01f*/);
	virtual ~GIRenderer() noexcept {}

	void set_deferred_rendering_enabled(bool enabled);

	void add_task(const gpu_task::TaskPtr &t);
	void remove_task(const gpu_task::TaskPtr &t);
	void add_gui_task(const gpu_task::TaskPtr &t);
	void remove_gui_task(const gpu_task::TaskPtr &t);

	virtual void render_queue() override;

	// const dense_voxel_space& voxel_grid() const { return voxel_space; }

	auto *get_gbuffer() const { return &gbuffer; }
	auto &get_scene_task() const { return scene_task; }

	void attach_profiler(profiler *p) { q.attach_profiler(p); }

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}
