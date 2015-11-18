// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "deferred_fbo.h"
#include "Quad.h"
#include "renderable.h"
#include "rendering_system.h"

#include "SceneProperties.h"
#include "light.h"

#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"

#include <memory>

namespace StE {
namespace Graphics {

class GIRenderer : public rendering_system {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	class deferred_composition : public renderable {
	private:
		GIRenderer *dr;

	public:
		deferred_composition(const StEngineControl &ctx, GIRenderer *dr) : renderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "deferred.vert", "deferred.frag" })()), dr(dr) {}

		virtual void prepare() const override {
			renderable::prepare();
			dr->fbo.bind_output_textures();
			dr->scene_props->material_storage().buffer().bind(LLR::shader_storage_layout_binding(0));
			dr->scene_props->lights_storage().bind_buffers(1);
			ScreenFillingQuad.vao()->bind();
		}

		virtual void render() const override {
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	};

private:
	deferred_fbo fbo;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	
	deferred_composition composer;
	rendering_queue ppq;

	SceneProperties *scene_props;

public:
	GIRenderer(const StEngineControl &ctx, SceneProperties *props) : fbo(ctx.get_backbuffer_size()), composer(ctx, this), scene_props(props) {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			this->fbo.resize(size);
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
	}
	virtual ~GIRenderer() noexcept {}

	auto get_fbo() const { return fbo.get_fbo(); }
	auto z_buffer() const { return fbo.z_buffer(); }

	void set_model_matrix(const glm::mat4 &m) {
		composer.get_program()->set_uniform("view_matrix", m);
	}

	void set_output_fbo(const LLR::GenericFramebufferObject *ofbo) {
		composer.set_output_fbo(ofbo);
	}

	virtual void finalize_queue(const StEngineControl &ctx) override {
		queue().push_back(&composer);

		scene_props->pre_draw();
	}

	virtual void render_queue(const StEngineControl &ctx) override {
		queue().render(get_fbo());
		ppq.render(&ctx.gl()->defaut_framebuffer());

		scene_props->post_draw();
	}

	rendering_queue& postprocess_queue() { return ppq; };

	virtual std::string rendering_system_name() const override { return "GIRenderer"; };
};

}
}
