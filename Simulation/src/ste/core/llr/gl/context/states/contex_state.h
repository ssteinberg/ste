// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "tuple_type_erasure.h"

#include "optional.h"

#include <functional>
#include <list>

namespace StE {
namespace LLR {

class context_state {
private:
	using StateSetterFunc = std::function<void(const tuple_type_erause&)>;
	using StateT = std::pair<tuple_type_erasure, StateSetterFunc>;

private:
	optional<StateT> state;
	std::list<StateT> stack;

public:
	context_state(StateSetterFunc &&f) : setter(std::move(f)) {}
	
	context_state(context_state &&) = default;
	context_state &operator=(context_state &&) = default;

	bool exists() const {
		return !!state;
	}
	
	template <typename... Args>
	bool compare(Args... &&args) const {
		if (!exists())
			return false;
		return state.get().first.compare_weak(std::forward<Args>(args)...);
	}
	template <typename... Ts>
	bool compare(std::tuple<Ts...> &&t) const {
		if (!exists())
			return false;
		return state.get().first.compare_weak(std::move(t));
	}
	
	template <typename... Args>
	void set(StateSetterFunc &&f, Args... &&args) {
		state = std::make_pair(std::move(f), tuple_type_erasure{ std::forward<Args>(args)... });
	}
	template <typename... Ts>
	void set(StateSetterFunc &&f, std::tuple<Ts...> &&t) {
		states = std::make_pair(std::move(f), tuple_type_erasure{ std::move(t) });
	}
	
	void push() {
		exists() ?
			stack.push_front(state.get()) :
			stack.push_front(StateT());
	}
	
	optional<StateT> pop() {
		if (stack.size() > 0) {
			state = stack.begin();
			stack.pop_front();
			return state;
		}
		return none;
	}
	
	template <typename... Ts>
	auto get() const {
		if (exists())
			return state.get().first.get_weak<Ts...>();
		return std::tuple<Ts...>();
	}
	
	tuple_type_erasure get_state() const {
		if (exists())
			return state;
		return StateT();
	}
};
	
}
}
