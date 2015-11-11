// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "deferred_fbo.h"
#include "Quad.h"
#include "renderable.h"
#include "rendering_system.h"

#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"

#include <memory>

namespace StE {
namespace Graphics {

class DeferredRenderer : public rendering_system {
private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

	class deferred_composition : public renderable {
	private:
		deferred_fbo *fbo;

	public:
		deferred_composition(const StEngineControl &ctx, deferred_fbo *fbo) : renderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert", "deferred.frag" })()), fbo(fbo) {}

		virtual void prepare() const override {
			renderable::prepare();
			fbo->bind_output_textures();
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

public:
	DeferredRenderer(const StEngineControl &ctx) : fbo(ctx.get_backbuffer_size()), composer(ctx, &fbo) {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			this->fbo.resize(size);
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
	}
	virtual ~DeferredRenderer() noexcept {}

	auto get_fbo() const { return fbo.get_fbo(); }
	auto z_buffer() const { return fbo.z_buffer(); }

	void set_output_fbo(const LLR::GenericFramebufferObject *ofbo) {
		composer.set_output_fbo(ofbo);
	}

	virtual void finalize_queue(const StEngineControl &ctx) override {
		queue().push_back(&composer);
	}

	virtual void render_queue(const StEngineControl &ctx) override {
		queue().render(get_fbo());
		ppq.render(&ctx.gl()->defaut_framebuffer());
	}

	rendering_queue& postprocess_queue() { return ppq; };

	void set_light_diffuse(const glm::vec3 &d) const { composer.get_program()->set_uniform("light_diffuse", d); }
	void set_light_luminance(float l) const { composer.get_program()->set_uniform("light_luminance", l); }
	void set_light_radius(float f) const { composer.get_program()->set_uniform("light_radius", f); }
	void set_light_pos(const glm::vec3 &p) const { composer.get_program()->set_uniform("light_pos", p); }
};

}
}
