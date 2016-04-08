
#include "stdafx.hpp"
#include "gpu_state_transition.hpp"
#include "gpu_state_transition_detail.hpp"

#include "gl_current_context.hpp"

#include <algorithm>

using namespace StE::Graphics;

StE::Core::gl_virtual_context gpu_state_transition::virt_ctx;

std::size_t gpu_state_transition::setup_virtual_context_and_calculate_transition(const gpu_task *task,
																				 const gpu_task *next,
																				 std::vector<Core::context_state_name> &states_to_push,
																				 std::vector<Core::context_state_name> &states_to_pop,
																				 _state_transition::state_container<Core::context_state> &states_to_set) {
	auto ctx = Core::gl_current_context::get();

	virt_ctx.make_current();
	virt_ctx.clear();

	task->set_context_state();
	auto states_intermediate = virt_ctx.get_states();
	auto states_resources = virt_ctx.get_resources();

	virt_ctx.clear();

	next->set_context_state();
	auto &final_states = virt_ctx.get_states();

	ctx->make_current();

	std::size_t cost = 0;
	_state_transition::states_diff(std::move(states_intermediate), final_states, states_to_push, states_to_pop, states_to_set, cost);
	for (auto &p : states_resources) {
		assert(p.second.exists());
		states_to_set.insert(std::move(p.second));
		cost += _state_transition::cost_for_state_type(Core::context_state_type_from_name(p.first.get_name()));
	}

	return cost;
}

std::unique_ptr<gpu_state_transition> gpu_state_transition::transition_function(const gpu_task *task, const gpu_task *next) {
	std::vector<Core::context_state_name> states_to_pop, states_to_push;
	_state_transition::state_container<Core::context_state> states_to_set;
	auto cost = setup_virtual_context_and_calculate_transition(task, next, states_to_push, states_to_pop, states_to_set);

	std::function<void(void)> dispatch = [states_to_push = std::move(states_to_push),
										  states_to_pop  = std::move(states_to_pop),
										  states_to_set  = std::move(states_to_set),
										  task, next]() {
		// Set new states
		for (auto &state : states_to_set) {
			assert(state.exists());
			Core::gl_current_context::get()->set_context_server_state(state.get_state());
		}

		// Dispatch
		task->dispatch();

		// Reset states used by task and unused by next
		for (auto &k : states_to_pop)
			Core::gl_current_context::get()->pop_state(k);
		// Push states used by next and unusedby task
		for (auto &k : states_to_push)
			Core::gl_current_context::get()->push_state(k);
	};

	return std::make_unique<gpu_state_transition>(AccessToken(),
												  std::move(dispatch),
												  cost,
												  task, next);
}
