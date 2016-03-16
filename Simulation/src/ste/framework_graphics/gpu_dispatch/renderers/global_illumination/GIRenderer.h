// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "Camera.h"

#include "deferred_fbo.h"
#include "Quad.h"
#include "gpu_task.h"
#include "gpu_task_dispatch_queue.h"

#include "Scene.h"
#include "SceneProperties.h"
#include "light.h"
#include "hdr_dof_postprocess.h"

#include "dense_voxel_space.h"
#include "fb_clear_dispatch.h"

#include "GLSLProgram.h"
#include "GLSLProgramFactory.h"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer : public rendering_system {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	class deferred_composition : public gpu_task {
		using Base = gpu_task;
		
		friend class GIRenderer;
		
	private:
		GIRenderer *dr;
		std::shared_ptr<LLR::GLSLProgram> program;

	public:
		deferred_composition(const StEngineControl &ctx, GIRenderer *dr) : program(ctx.glslprograms_pool().fetch_program_task({ "deferred.vert", "deferred.frag" })()), dr(dr) {
			// dr->voxel_space.add_consumer_program(this->get_program());
		}
		~deferred_composition() {
			// dr->voxel_space.remove_consumer_program(this->get_program());
		}

	protected:
		void set_context_state() const override final {
			using namespace LLR;
			
			Base::set_context_state();
		
			dr->fbo.bind_output_textures();
			dr->scene->scene_properties().material_storage().buffer().bind(shader_storage_layout_binding(0));
			dr->scene->scene_properties().lights_storage().bind_buffers(1);
			ScreenFillingQuad.vao()->bind();
			
			program->bind();
		}

		void dispatch() const override final {
			dr->scene->scene_properties().pre_draw();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			dr->scene->scene_properties().post_draw();
		}
	};

private:
	deferred_fbo fbo;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

	const StEngineControl &ctx;
	Scene *scene;
	// dense_voxel_space voxel_space;
	
	gpu_task::TasksCollection gui_tasks;
	
	std::shared_ptr<hdr_dof_postprocess> hdr;
	std::shared_ptr<deferred_composition> composer;
	std::shared_ptr<StE::Graphics::fb_clear_dispatch<false>> fb_clearer;

protected:
	auto get_fbo() const { return fbo.get_fbo(); }

public:
	GIRenderer(const StEngineControl &ctx, 
			   Scene *scene/*,
			   std::size_t voxel_grid_size = 512, 
			   float voxel_grid_ratio = .01f*/) 
			   	: fbo(ctx.get_backbuffer_size()), scene(scene), 
				  ctx(ctx),
				  //voxel_space(ctx, voxel_grid_size, voxel_grid_ratio), 
				  hdr(std::make_shared<hdr_dof_postprocess>(ctx, fbo.z_buffer())), 
				  composer(std::make_shared<deferred_composition>(ctx, this)),
				  fb_clearer(std::make_shared<StE::Graphics::fb_clear_dispatch<false>>()) {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			this->fbo.resize(size);
			hdr->set_z_buffer(fbo.z_buffer());
			
			for (auto &task_ptr : gui_tasks)
				q.update_task_fbo(task_ptr, &this->ctx.gl()->defaut_framebuffer());
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);

		composer->program->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));

		// queue().push_back(voxel_space.voxelizer(*scene));
		hdr->add_dependency(composer);
		q.add_task(hdr);
		q.add_task(composer, hdr->get_input_fbo());
		add_task(fb_clearer);
	}
	virtual ~GIRenderer() noexcept {}

	void update_model_matrix_from_camera(const Camera &camera) {
		glm::mat4 m = camera.view_matrix();

		composer->program->set_uniform("inv_view_model", glm::inverse(m));
		composer->program->set_uniform("view_matrix", m);

		// voxel_space.set_model_matrix(m, camera.get_position());
	}
	
	void add_task(const gpu_task::TaskPtr &t) {
		composer->add_dependency(t);
		t->add_dependency(fb_clearer);
		q.add_task(t, get_fbo());
	}
	
	void remove_task(const gpu_task::TaskPtr &t) {
		composer->remove_dependency(t);
		t->remove_dependency(fb_clearer);
		q.remove_task(t);
	}
	
	void add_gui_task(const gpu_task::TaskPtr &t) {
		t->add_dependency(hdr);
		q.add_task(t, &ctx.gl()->defaut_framebuffer());
		
		gui_tasks.insert(t);
	}
	
	void remove_gui_task(const gpu_task::TaskPtr &t) {
		t->remove_dependency(hdr);
		q.remove_task(t);
		
		gui_tasks.erase(t);
	}

	virtual void render_queue(const StEngineControl &ctx) override {
		// queue().render(get_fbo());
		// ppq.render(&ctx.gl()->defaut_framebuffer());
		q.dispatch();
	}
	// const dense_voxel_space& voxel_grid() const { return voxel_space; }

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}
