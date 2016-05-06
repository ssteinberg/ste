// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "context_state.hpp"
#include "context_state_name.hpp"
#include "context_state_key.hpp"
#include "context_state_type.hpp"

#include <vector>
#include <algorithm>

namespace StE {
namespace Core {
namespace GL {

class gl_context_state_log {
public:
	static constexpr unsigned state_change_cost = 1;
	static constexpr unsigned buffer_change_cost = 3;
	static constexpr unsigned texture_change_cost = 20;
	static constexpr unsigned shader_change_cost = 15;
	static constexpr unsigned fbo_change_cost = 10;
	static constexpr unsigned va_change_cost = 10;
	static constexpr unsigned transform_feedback_cost = 8;

	template <typename T>
	using container = std::vector<T>;
	using states_value_type = std::pair<context_state_key<StateName>, optional<context_state::state_type>>;

private:
	inline std::size_t cost_for_state_type(const Core::GL::context_state_type &t) {
		switch (t) {
			case Core::GL::context_state_type::BasicState:
				return state_change_cost;
			case Core::GL::context_state_type::BufferChange:
				return buffer_change_cost;
			case Core::GL::context_state_type::TextureChange:
				return texture_change_cost;
			case Core::GL::context_state_type::FBOChange:
				return fbo_change_cost;
			case Core::GL::context_state_type::ProgramChange:
				return shader_change_cost;
			case Core::GL::context_state_type::VAOChange:
				return va_change_cost;
			case Core::GL::context_state_type::TransformFeedbackActivation:
				return transform_feedback_cost;
		}

		assert(false);
		return 1;
	}

private:
	container<BasicStateName> basic_states;
	container<states_value_type> states;
	container<context_state_key<StateName>> resources;

	int cost{ 0 };

public:
	gl_context_state_log() {
		basic_states.reserve(10);
		states.reserve(10);
		resources.reserve(10);
	}

	inline void log_basic_state_change(bool executed, const decltype(basic_states)::value_type &s) {
		basic_states.push_back(s);
		if (executed)
			cost += cost_for_state_type(context_state_type_from_name(s));
	}
	inline void log_state_change(bool executed, const decltype(states)::value_type &s) {
		states.push_back(s);
		if (executed)
			cost += cost_for_state_type(context_state_type_from_name(s.first.get_name()));
	}
	inline void log_resource_change(bool executed, const decltype(resources)::value_type &s) {
		resources.push_back(s);
		if (executed)
			cost += cost_for_state_type(context_state_type_from_name(s.get_name()));
	}

	void clear() {
		cost = 0;
		basic_states.clear();
		states.clear();
		resources.clear();
	}

	auto &get_basic_states() const { return basic_states; }
	auto &get_basic_states() { return basic_states; }
	auto &get_states() const { return states; }
	auto &get_states() { return states; }
	auto &get_resources() const { return resources; }
	auto &get_resources() { return resources; }

	void swap(gl_context_state_log &l) {
		std::swap(l.basic_states, basic_states);
		std::swap(l.states, states);
		std::swap(l.resources, resources);
	}

	auto get_cost() const { return cost; }
};

}
}
}

namespace std {
	template<>
	void inline swap(StE::Core::GL::gl_context_state_log& lhs, StE::Core::GL::gl_context_state_log& rhs) {
		lhs.swap(rhs);
	}
}
