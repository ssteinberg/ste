// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "Sampler.hpp"

#include "glsl_program.hpp"
#include "deferred_gbuffer.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class gbuffer_clear_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<gbuffer_clear_dispatch>;
	friend class Resource::resource_instance<gbuffer_clear_dispatch>;

private:
	mutable deferred_gbuffer *gbuffer;

public:
	gbuffer_clear_dispatch(const StEngineControl &ctx,
						   deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {}

	void set_context_state() const override final {}
	void dispatch() const override final;
};

}
}
