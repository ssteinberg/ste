// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "optional.hpp"

#include <functional>
#include <vector>

namespace StE {
namespace Core {
namespace GL {

class context_basic_state {
public:
	using state_type = bool;

private:
	optional<state_type> state;
	std::vector<state_type> stack;

public:
	context_basic_state() = default;
	context_basic_state(context_basic_state &&) = default;
	context_basic_state &operator=(context_basic_state &&) = default;
	context_basic_state(const context_basic_state &) = default;
	context_basic_state &operator=(const context_basic_state &) = default;

	bool operator==(const context_basic_state &s) const {
		return exists() && s.exists() &&
			   state.get() == s.state.get();
	}
	bool operator!=(const context_basic_state &te) const {
		return !((*this) == te);
	}

	bool exists() const {
		return !!state;
	}

	bool compare(const state_type &t) const {
		if (!exists())
			return false;
		return state.get() == t;
	}

	void set(const state_type &s) {
		state = s;
	}

	void push() {
		assert(exists() && "State must be set before pushing.");
		exists() ?
			stack.push_back(state.get()) :
			stack.push_back(state_type());
		assert(stack.size() < 20);
	}

	optional<state_type> pop() {
		if (stack.size() > 0) {
			state = stack.back();
			stack.pop_back();
			return state;
		}
		return none;
	}

	auto stack_size() const {
		return stack.size();
	}

	auto &get_state() const {
		assert(exists() && "State must be set before calling get_state");
		return state.get();
	}
};

}
}
}
