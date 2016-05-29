// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"
#include "gl_current_context.hpp"

#include "ssss_generator.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "glsl_program.hpp"

#include "Quad.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_bilateral_blur_y : public gpu_dispatchable {
	friend class Resource::resource_loading_task<ssss_bilateral_blur_y>;

private:
	const ssss_generator *p;
	Resource::resource_instance<Core::glsl_program> ssss_blur_program;

public:
	ssss_bilateral_blur_y(const StEngineControl &ctx, const ssss_generator *p) : p(p) {
		ssss_blur_program.load(ctx, std::vector<std::string>{ "ssss_blur.vert", "ssss_blur.geom", "ssss_blur_y.frag" });
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::ssss_bilateral_blur_y> {
	using R = Graphics::ssss_bilateral_blur_y;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->ssss_blur_program.wait();

			return object;
		});
	}
};

}
}
