// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

namespace StE {
namespace Graphics {

class light_preprocessor;

class light_preprocess_cull_shadows : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const light_preprocessor *lp;

public:
	light_preprocess_cull_shadows(const light_preprocessor *lp) : lp(lp) {}

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}
}
