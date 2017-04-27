// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>

#include <signal.hpp>

#include <gpu_dispatchable.hpp>

#include <glsl_program.hpp>
#include <texture_2d.hpp>
#include <sampler.hpp>
#include <framebuffer_object.hpp>

#include <memory>

namespace ste {
namespace Graphics {

class fxaa_dispatchable : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class resource::resource_loading_task<fxaa_dispatchable>;
	friend class resource::resource_instance<fxaa_dispatchable>;

private:
	using ResizeSignalConnectionType = ste_engine_control::framebuffer_resize_signal_type::connection_type;

private:
	resource::resource_instance<resource::glsl_program> program;
	Core::framebuffer_object fbo;
	std::unique_ptr<Core::texture_2d> input;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

private:
	void resize(const glm::ivec2 &size) {
		input = std::make_unique<Core::texture_2d>(gli::format::FORMAT_RGB16_SFLOAT_PACK16, ste::Core::texture_2d::size_type(size), 1);
		fbo[0] = *input;

		auto handle = input->get_texture_handle(*Core::sampler::sampler_linear_clamp());
		handle.make_resident();
		program.get().set_uniform("input_tex", handle);
	}

public:
	fxaa_dispatchable(const ste_engine_control &ctx) : program(ctx, std::vector<std::string>{ "fxaa.vert", "fxaa.frag" }) {
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

namespace resource {

template <>
class resource_loading_task<Graphics::fxaa_dispatchable> {
	using R = Graphics::fxaa_dispatchable;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object, &ctx]() {
			object->resize(ctx.get_backbuffer_size());
		});;
	}
};

}
}
