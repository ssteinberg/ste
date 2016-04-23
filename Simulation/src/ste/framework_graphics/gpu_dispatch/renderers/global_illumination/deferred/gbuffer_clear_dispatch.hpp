// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "deferred_gbuffer.hpp"

namespace StE {
namespace Graphics {

class gbuffer_clear_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	deferred_gbuffer *gbuffer;

public:
	gbuffer_clear_dispatch(deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {}

protected:
	virtual void set_context_state() const override {}

	virtual void dispatch() const override {
		gbuffer->clear();
	}
};

}
}
