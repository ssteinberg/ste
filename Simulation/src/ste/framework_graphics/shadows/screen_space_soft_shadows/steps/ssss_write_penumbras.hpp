// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "ssss_generator.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "glsl_program.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_write_penumbras : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<ssss_write_penumbras>;

private:
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const ssss_generator *p;
	Resource::resource_instance<Core::glsl_program> ssss_gen_program;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	ssss_write_penumbras(const StEngineControl &ctx, const ssss_generator *p) : p(p) {
		ssss_gen_program.load(ctx, "ssss.glsl");

		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](float, float ffov, float fnear) {
			ssss_gen_program.get().set_uniform("near", fnear);
			ssss_gen_program.get().set_uniform("half_over_tan_fov_over_two", .5f / glm::tan(ffov * .5f));
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::ssss_write_penumbras> {
	using R = Graphics::ssss_write_penumbras;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->ssss_gen_program.wait();

			object->ssss_gen_program.get().set_uniform("near", ctx.get_near_clip());
			object->ssss_gen_program.get().set_uniform("half_over_tan_fov_over_two", .5f / glm::tan(ctx.get_fov() * .5f));

			return object;
		});
	}
};

}
}
