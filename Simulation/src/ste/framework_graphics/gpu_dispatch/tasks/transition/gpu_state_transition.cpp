
#include "stdafx.hpp"
#include "gpu_state_transition.hpp"

#include "gpu_dispatchable.hpp"

#include "gl_context_state_log.hpp"
#include "gl_current_context.hpp"

#include <algorithm>

using namespace StE::Graphics;

namespace StE {
namespace detail {

inline void set_difference(StE::Core::GL::gl_context_state_log::container<StE::Core::GL::BasicStateName> &a,
						   const StE::Core::GL::gl_context_state_log::container<StE::Core::GL::BasicStateName> &b) {
	if (!a.size())
		return;

	auto *k = &a[0];
	for (int i = 0; i < a.size();) {
		auto it = std::find_if(b.begin(), b.end(), [=](const StE::Core::GL::BasicStateName &v){
			return v == *k;
		});
		if (it != b.end())
			a.erase(a.begin() + i, a.begin() + i + 1);
		else
			++i; ++k;
	}
}

inline void set_difference(StE::Core::GL::gl_context_state_log::container<StE::Core::GL::gl_context_state_log::states_value_type> &a,
						   StE::Core::GL::gl_context_state_log::container<StE::Core::GL::gl_context_state_log::states_value_type> &b) {
	if (!a.size())
		return;

	auto *k = &a[0];
	for (int i = 0; i < a.size();) {
		auto it = std::find_if(b.begin(), b.end(), [=](const StE::Core::GL::gl_context_state_log::states_value_type &v){
			return v.first == k->first;
		});
		if (it != b.end()) {
			it->second = std::move(k->second);
			a.erase(a.begin() + i, a.begin() + i + 1);
		}
		else
			++i; ++k;
	}
}

}
}

void gpu_state_transition::log_and_set_task_state(const gpu_task *task) const {
	task->dispatchable->context_state_descriptor.clear();

	Core::GL::gl_current_context::get()->attach_logger(&task->dispatchable->context_state_descriptor);
	task->set_context_state();
	Core::GL::gl_current_context::get()->detach_logger();
}

StE::Core::GL::gl_context_state_log gpu_state_transition::state_to_pop(const gpu_task *prev_task, const gpu_task *task) const {
	auto &l_prev = prev_task->dispatchable->context_state_descriptor;
	auto &l = task->dispatchable->context_state_descriptor;

	Core::GL::gl_context_state_log to_pop;
	std::swap(to_pop, l_prev);

	detail::set_difference(to_pop.get_basic_states(), l.get_basic_states());
	detail::set_difference(to_pop.get_states(), l.get_states());

	return to_pop;
}

void gpu_state_transition::update_weight_and_transition() const {
	const gpu_task *prev_task = reinterpret_cast<const gpu_task*>(this->get_from());
	const gpu_task *task = reinterpret_cast<const gpu_task*>(this->get_to());

	log_and_set_task_state(task);
	auto pop_descriptor = state_to_pop(prev_task, task);

	Core::GL::gl_current_context::get()->restore_states_from_logger(pop_descriptor);

	task->dispatch();

	auto cost = task->dispatchable->context_state_descriptor.get_cost() + pop_descriptor.get_basic_states().size() + pop_descriptor.get_states().size();
	set_weight(cost);
}
