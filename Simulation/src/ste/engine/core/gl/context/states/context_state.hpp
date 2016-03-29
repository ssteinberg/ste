// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "tuple_type_erasure.hpp"

#include "optional.hpp"

#include <functional>
#include <list>

namespace StE {
namespace Core {

class context_state {
private:
	struct state_type {
		using StateSetterFunc = std::function<void(const tuple_type_erasure&)>;
		
		tuple_type_erasure value, args;
		StateSetterFunc setter;
	};

private:
	optional<state_type> state;
	std::list<state_type> stack;

public:
	context_state() = default;
	context_state(context_state &&) = default;
	context_state &operator=(context_state &&) = default;
	context_state(const context_state &) = default;
	context_state &operator=(const context_state &) = default;

	bool exists() const {
		return !!state;
	}
	
	template <typename... Ts>
	bool compare(const std::tuple<Ts...> &t) const {
		if (!exists())
			return false;
		return state.get().value.compare_weak(t);
	}
	
	template <typename... Ts, typename... Args>
	void set(state_type::StateSetterFunc &&f, const std::tuple<Ts...> &v, const std::tuple<Args...> &args) {
		state = state_type{ tuple_type_erasure{ v }, tuple_type_erasure{ args }, std::move(f) };
	}
	
	void push() {
		assert(exists() && "State must be set before pushing.");
		exists() ?
			stack.push_front(state.get()) :
			stack.push_front(state_type());
	}
	
	optional<state_type> pop() {
		if (stack.size() > 0) {
			state = *stack.begin();
			stack.pop_front();
			return state;
		}
		return none;
	}
	
	template <typename... Ts>
	auto get_value() const {
		assert(exists() && "State must be set before calling get<>.");
		if (exists())
			return state.get().value.get_weak<Ts...>();
		return std::tuple<Ts...>();
	}
	
	template <typename... Ts>
	auto get_args() const {
		assert(exists() && "State must be set before calling get<>.");
		if (exists())
			return state.get().args.get_weak<Ts...>();
		return std::tuple<Ts...>();
	}
	
	auto &get() const {
		return state;
	}
};
	
}
}
