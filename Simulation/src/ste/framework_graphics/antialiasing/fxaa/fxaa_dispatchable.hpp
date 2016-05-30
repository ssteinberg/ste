// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program.hpp"

#include "Texture2D.hpp"
#include "FramebufferObject.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class fxaa_dispatchable : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<fxaa_dispatchable>;

private:
	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	Resource::resource_instance<Resource::glsl_program> program;
	Core::FramebufferObject fbo;
	std::unique_ptr<Core::Texture2D> input;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

private:
	void resize(const glm::ivec2 &size) {
		input = std::make_unique<Core::Texture2D>(gli::format::FORMAT_RGB16_SFLOAT_PACK16, StE::Core::Texture2D::size_type(size), 1);
		fbo[0] = *input;
	}

public:
	fxaa_dispatchable(const StEngineControl &ctx) : program(ctx, std::vector<std::string>{ "fxaa.vert", "fxaa.frag" }) {
		resize(ctx.get_backbuffer_size());
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			resize(size);
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
	}
	~fxaa_dispatchable() noexcept {}

	auto get_input_fbo() const { return &fbo; }

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::fxaa_dispatchable> {
	using R = Graphics::fxaa_dispatchable;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		});
	}
};

}
}
