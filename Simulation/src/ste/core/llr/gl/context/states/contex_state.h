// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "tuple_type_erasure.h"

#include "optional.h"

#include <functional>

namespace StE {
namespace LLR {

class context_state {
private:
	optional<tuple_type_erasure> state, old_state;
	std::function<void(void)> state_setter, old_state_setter;

public:
	bool exists() const {
		return !!state;
	}
	
	template <typename... Args>
	bool compare(Args... args) const {
		if (!state)
			return false;
		return state.get().compare_weak(args...);
	}
	template <typename... Ts>
	bool compare(std::tuple<Ts...> &&t) const {
		if (!state)
			return false;
		return state.get().compare_weak(std::move(t));
	}
	
	template <typename... Args>
	void push(std::function<void(void)> &&setter, Args... args) {
		old_state = std::move(state);
		state = tuple_type_erasure{ args... };
		old_state_setter = std::move(state_setter); 
		state_setter = std::move(setter);
	}
	template <typename... Ts>
	void push(std::function<void(void)> &&setter, std::tuple<Ts...> &&t) {
		old_state = std::move(state);
		states = tuple_type_erasure{ std::move(t) };
		old_state_setter = std::move(state_setter); 
		state_setter = std::move(setter);
	}
	
	template <typename... Ts>
	auto get() const {
		if (exists())
			return state.get().get_weak<Ts...>();
		return std::tuple<Ts...>();
	}
	
	auto pop() {
		if (old_state) {
			auto ret = old_state_setter;
			
			state = std::move(old_state);
			state_setter = std::move(old_state_setter);
			
			return ret;
		}
	}
	
	auto get_state_setter() const { return state_setter; }
	auto get_old_state_setter() const { return old_state_setter; }
};
	
class context_basic_state {
private:
	optional<bool> state, old_state;

public:
	bool exists() const {
		return !!state;
	}
	
	bool old_exists() const {
		return !!old_state;
	}
	
	bool compare(bool b) const {
		if (!state)
			return false;
		return state.get() == b;
	}
	
	void push(bool b) {
		old_state = std::move(state);
		state = b;
	}
	
	bool pop() {
		if (old_state) {
			auto ret = old_state.get();
			state = std::move(old_state);
			return ret;
		}
	}
	
	bool get() const {
		assert(exists());
		return state.get();
	}
	
	bool get_old() const {
		assert(old_exists());
		return old_state.get();
	}
};
	
}
}
