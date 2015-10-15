// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class gl_state {
private:
	friend class gl_state_manager;
	friend struct std::hash<gl_state>;

	gl_state(GLenum state) : state(state), enabled(false) {}

public:
	const bool enabled;
	const GLenum state;

public:
	gl_state(GLenum state, bool enabled) : state(state), enabled(enabled) {}

	bool operator==(const gl_state &rhs) const { return state == rhs.state; }
	bool operator<(const gl_state &rhs) const { return state < rhs.state; }
};

}
}

namespace std {

template <> struct hash<StE::LLR::gl_state> {
	size_t inline operator()(const StE::LLR::gl_state &x) const {
		return std::hash<GLenum>()(x.state);
	}
};

}
