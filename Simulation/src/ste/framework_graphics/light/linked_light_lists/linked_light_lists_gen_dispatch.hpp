// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "linked_light_lists.hpp"
#include "light_storage.hpp"

#include "GLSLProgram.hpp"
#include "Texture2D.hpp"
#include "Sampler.hpp"

namespace StE {
namespace Graphics {

class linked_light_lists_gen_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const StEngineControl &ctx;
	light_storage *ls;
	linked_light_lists *lll;
	std::shared_ptr<Core::GLSLProgram> program;

	Core::SamplerMipmapped depth_sampler;

	Core::Texture2D *depth_map;

private:
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;

	void update_uniforms() const {
		float fovy = ctx.get_fov();
		float n = ctx.get_near_clip();

		program->set_uniform("near", n);
		program->set_uniform("aspect", ctx.get_projection_aspect());
		program->set_uniform("two_near_tan_fovy_over_two", 2.f * n * glm::tan(fovy * .5f));
		program->set_uniform("backbuffer_size", glm::vec2(ctx.get_backbuffer_size()));
	}

public:
	linked_light_lists_gen_dispatch(const StEngineControl &ctx,
									light_storage *ls,
									linked_light_lists *lll) : ctx(ctx), ls(ls), lll(lll),
															   program(ctx.glslprograms_pool().fetch_program_task({ "passthrough.vert", "linked_light_lists_gen.frag" })()),
															   depth_sampler(Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest,
															   				 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		update_uniforms();
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](float, float, float) {
			update_uniforms();
		});
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			update_uniforms();
		});
		ctx.signal_projection_change().connect(projection_change_connection);
		ctx.signal_framebuffer_resize().connect(resize_connection);
	}

	void set_depth_map(Core::Texture2D *dm) { depth_map = dm; }

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}
}
