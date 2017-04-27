// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>

#include <signal.hpp>

#include <light_storage.hpp>
#include <glsl_program.hpp>

#include <gpu_dispatchable.hpp>

namespace ste {
namespace Graphics {

class light_preprocessor : public gpu_dispatchable {
	friend class light_preprocess_cull_lights;
	friend class light_preprocess_cull_shadows;

	friend class resource::resource_loading_task<light_preprocessor>;
	friend class resource::resource_instance<light_preprocessor>;

private:
	using ResizeSignalConnectionType = ste_engine_control::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = ste_engine_control::projection_change_signal_type::connection_type;

private:
	const ste_engine_control &ctx;
	light_storage *ls;

	resource::resource_instance<resource::glsl_program> light_preprocess_cull_lights_program;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;

private:
	light_preprocessor(const ste_engine_control &ctx,
					   light_storage *ls) : ctx(ctx), ls(ls),
											light_preprocess_cull_lights_program(ctx, "light_preprocess_cull_lights.comp") {
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			set_projection_planes();
		});
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](float, float, float n) {
			set_projection_planes();
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
		ctx.signal_projection_change().connect(projection_change_connection);
	}

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}

namespace resource {

template <>
class resource_loading_task<Graphics::light_preprocessor> {
	using R = Graphics::light_preprocessor;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->light_preprocess_cull_lights_program.wait();
			// TODO: Fix
		}).then/*_on_main_thread*/([object]() {
			object->set_projection_planes();
		});
	}
};

}
}
