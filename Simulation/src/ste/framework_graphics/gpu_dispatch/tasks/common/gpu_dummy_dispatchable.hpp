// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

namespace StE {
namespace Graphics {

class gpu_dummy_dispatchable : public gpu_dispatchable {
protected:
	void set_context_state() const override final {}
	void dispatch() const override final {}
};

}
}
