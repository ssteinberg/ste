// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "context_state_type.hpp"

#include <vector>
#include <unordered_map>

namespace StE {
namespace Graphics {
namespace _state_transition {

static constexpr unsigned state_change_cost = 1;
static constexpr unsigned buffer_change_cost = 3;
static constexpr unsigned texture_change_cost = 20;
static constexpr unsigned shader_change_cost = 15;
static constexpr unsigned fbo_change_cost = 10;
static constexpr unsigned va_change_cost = 10;
static constexpr unsigned transform_feedback_cost = 8;

inline std::size_t cost_for_state_type(const Core::context_state_type &t) {
	switch (t) {
		case Core::context_state_type::BasicState:
			return state_change_cost;
		case Core::context_state_type::BufferChange:
			return buffer_change_cost;
		case Core::context_state_type::TextureChange:
			return texture_change_cost;
		case Core::context_state_type::FBOChange:
			return fbo_change_cost;
		case Core::context_state_type::ProgramChange:
			return shader_change_cost;
		case Core::context_state_type::VAOChange:
			return va_change_cost;
		case Core::context_state_type::TransformFeedbackActivation:
			return transform_feedback_cost;
	}

	assert(false);
	return 1;
}

template <typename K, typename V>
inline void states_diff(std::unordered_map<K,V> &&intermediate,
						const std::unordered_map<K,V> &final_states,
						std::vector<K> &states_to_push,
						std::vector<K> &states_to_pop,
						std::vector<V> &states_to_set,
						std::size_t &cost) {
	states_to_push.reserve(final_states.size());
	states_to_pop.reserve(intermediate.size());
	states_to_set.reserve(intermediate.size());

	for (auto &p : intermediate) {
		assert(p.second.exists());
		states_to_set.push_back(p.second);
		cost += cost_for_state_type(Core::context_state_type_from_name(p.first));
	}

	for (auto &p : final_states) {
		auto it = intermediate.find(p.first);
		if (it == intermediate.end())
			states_to_push.push_back(p.first);
		else
			intermediate.erase(it);
	}

	for (auto &p : intermediate) {
		states_to_pop.push_back(p.first);
		cost += cost_for_state_type(Core::context_state_type_from_name(p.first));
	}
}

}
}
}