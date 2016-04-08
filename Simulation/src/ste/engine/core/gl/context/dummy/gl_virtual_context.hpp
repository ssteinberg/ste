// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gl_generic_context.hpp"

namespace StE {
namespace Core {

class gl_virtual_context : public gl_generic_context {
	using Base = gl_generic_context;

public:
	gl_virtual_context() : Base(true) {}

	const auto &get_states() const { return Base::states; }
	const auto &get_resources() const { return Base::resources; }

	void clear() {
		Base::states.clear();
		Base::resources.clear();

		Base::state_counter = 0;
	}
};

}
}
