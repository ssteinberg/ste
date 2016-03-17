// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "tuple_type_erasure.hpp"

#include "optional.hpp"

#include <functional>
#include <list>

namespace StE {
namespace LLR {

class context_state {
private:
	using StateSetterFunc = std::function<void(const tuple_type_erasure&)>;
	using StateT = std::pair<tuple_type_erasure, StateSetterFunc>;

private:
	optional<StateT> state;
	std::list<StateT> stack;

public:
	context_state() = default;
	context_state(context_state &&) = default;
	context_state &operator=(context_state &&) = default;

	bool exists() const {
		return !!state;
	}
	
	template <typename... Ts>
	bool compare(const std::tuple<Ts...> &t) const {
		if (!exists())
			return false;
		return state.get().first.compare_weak(t);
	}
	
	template <typename... Ts>
	void set(StateSetterFunc &&f, const std::tuple<Ts...> &t) {
		state = StateT{ tuple_type_erasure{ t }, std::move(f) };
	}
	
	void push() {
		assert(exists() && "State must be set before pushing.");
		exists() ?
			stack.push_front(state.get()) :
			stack.push_front(StateT());
	}
	
	optional<StateT> pop() {
		if (stack.size() > 0) {
			state = *stack.begin();
			stack.pop_front();
			return state;
		}
		return none;
	}
	
	template <typename... Ts>
	auto get() const {
		assert(exists() && "State must be set before calling get<>.");
		if (exists())
			return state.get().first.get_weak<Ts...>();
		return std::tuple<Ts...>();
	}
	
	auto get_state() const {
		return state;
	}
};
	
}
}
