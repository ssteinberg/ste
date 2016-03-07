// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "Camera.h"

#include "deferred_fbo.h"
#include "Quad.h"
#include "renderable.h"
#include "rendering_system.h"

#include "Scene.h"
#include "SceneProperties.h"
#include "light.h"
#include "hdr_dof_postprocess.h"

#include "dense_voxel_space.h"

#include "GLSLProgram.h"
#include "GLSLProgramFactory.h"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer : public rendering_system {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	class deferred_composition : public renderable {
		using Base = renderable;
		
	private:
		GIRenderer *dr;

	public:
		deferred_composition(const StEngineControl &ctx, GIRenderer *dr) : Base(StE::Resource::GLSLProgramFactory::load_program_task(ctx, { "deferred.vert", "deferred.frag" })()), dr(dr) {
			// dr->voxel_space.add_consumer_program(this->get_program());
		}
		~deferred_composition() {
			// dr->voxel_space.remove_consumer_program(this->get_program());
		}

		virtual void prepare() const override {
			Base::prepare();

			dr->scene_props->pre_draw();

			dr->fbo.bind_output_textures();
			dr->scene_props->material_storage().buffer().bind(LLR::shader_storage_layout_binding(0));
			dr->scene_props->lights_storage().bind_buffers(1);
			ScreenFillingQuad.vao()->bind();
		}

		virtual void render() const override {
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		virtual void finalize() const override {
			Base::finalize();
			
			dr->scene_props->post_draw();
		}
	};

private:
	deferred_fbo fbo;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	rendering_queue ppq;

	Scene *scene;
	SceneProperties *scene_props;
	hdr_dof_postprocess hdr;
	// dense_voxel_space voxel_space;
	
	deferred_composition composer;

protected:
	void set_output_fbo(const LLR::GenericFramebufferObject *ofbo) {
		composer.set_output_fbo(ofbo);
	}
	auto get_fbo() const { return fbo.get_fbo(); }

public:
	GIRenderer(const StEngineControl &ctx, 
			   Scene *scene,
			   SceneProperties *props/*,
			   std::size_t voxel_grid_size = 512, 
			   float voxel_grid_ratio = .01f*/) 
			   	: fbo(ctx.get_backbuffer_size()), scene(scene), scene_props(props), 
				  hdr(ctx, fbo.z_buffer()), 
				//   voxel_space(ctx, voxel_grid_size, voxel_grid_ratio), 
				composer(ctx, this) {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			this->fbo.resize(size);
			hdr.set_z_buffer(fbo.z_buffer());
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);

		this->set_output_fbo(hdr.get_input_fbo());

		composer.get_program()->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));
	}
	virtual ~GIRenderer() noexcept {}

	void update_model_matrix_from_camera(const Camera &camera) {
		glm::mat4 m = camera.view_matrix();

		composer.get_program()->set_uniform("inv_view_model", glm::inverse(m));
		composer.get_program()->set_uniform("view_matrix", m);

		// voxel_space.set_model_matrix(m, camera.get_position());
		scene->set_model_matrix(m);
	}

	virtual void finalize_queue(const StEngineControl &ctx) override {
		// queue().push_back(voxel_space.voxelizer(*scene));
		postprocess_queue().push_front(&hdr);
		postprocess_queue().push_front(&composer);
	}

	virtual void render_queue(const StEngineControl &ctx) override {
		queue().render(get_fbo());
		ppq.render(&ctx.gl()->defaut_framebuffer());
	}

	rendering_queue& postprocess_queue() { return ppq; }
	// const dense_voxel_space& voxel_grid() const { return voxel_space; }

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}
