// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "rendering_system.hpp"

#include "Camera.hpp"

#include "deferred_fbo.hpp"
#include "gpu_dispatchable.hpp"
#include "gpu_task.hpp"

#include "Scene.hpp"
#include "SceneProperties.hpp"
#include "light.hpp"

#include "gpu_dummy_dispatchable.hpp"
#include "hdr_dof_postprocess.hpp"
#include "ssss_storage.hpp"
#include "ssss_generator.hpp"

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
		GIRenderer *dr;
		std::shared_ptr<Core::GLSLProgram> program;

	public:
		deferred_composition(const StEngineControl &ctx, GIRenderer *dr) : program(ctx.glslprograms_pool().fetch_program_task({ "deferred.vert", "deferred.frag" })()), dr(dr) {
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
	using FbClearTask = StE::Graphics::fb_clear_dispatch<>;

private:
	deferred_fbo fbo;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

	const StEngineControl &ctx;
	std::shared_ptr<Scene> scene;
	// dense_voxel_space voxel_space;

private:
	gpu_task::TaskCollection gui_tasks;
	gpu_task::TaskCollection added_tasks;

	hdr_dof_postprocess hdr;
	ssss_storage ssss_layers;
	ssss_generator ssss_dispatchable;
	gpu_dummy_dispatchable precomposer_dummy_dispatchable;

	std::shared_ptr<const gpu_task> precomposer_dummy_task, composer_task, fb_clearer_task, ssss_task;

	deferred_composition composer;
	FbClearTask fb_clearer;

	bool use_deferred_rendering{ true };

protected:
	void rebuild_task_queue();

	const Core::GenericFramebufferObject *get_fbo() const {
		if (use_deferred_rendering)
			return fbo.get_fbo();
		return &ctx.gl()->defaut_framebuffer();
	}

public:
	GIRenderer(const StEngineControl &ctx,
			   const std::shared_ptr<Scene> &scene/*,
			   std::size_t voxel_grid_size = 512,
			   float voxel_grid_ratio = .01f*/);
	virtual ~GIRenderer() noexcept {}

	void set_model_matrix(const glm::mat4 &m) {
		composer.program->set_uniform("view_matrix", m);

		// voxel_space.set_model_matrix(m, camera.get_position());

		ssss_dispatchable.set_model_matrix(m);
	}

	void set_deferred_rendering_enabled(bool enabled);

	void add_task(const gpu_task::TaskPtr &t);
	void remove_task(const gpu_task::TaskPtr &t);
	void add_gui_task(const gpu_task::TaskPtr &t);
	void remove_gui_task(const gpu_task::TaskPtr &t);

	virtual void render_queue() override;

	// const dense_voxel_space& voxel_grid() const { return voxel_space; }

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}
