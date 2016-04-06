// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "gpu_task.hpp"
#include "sop_edge.hpp"

#include "gl_virtual_context.hpp"
#include "context_state_type.hpp"

#include <functional>
#include <vector>
#include <unordered_map>

#include <memory>

namespace StE {
namespace Graphics {

class gpu_state_transition : public Algorithm::SOP::sop_edge {
	using Base = Algorithm::SOP::sop_edge;

private:
	struct AccessToken {};

protected:
	static Core::gl_virtual_context virt_ctx;

private:
	static std::size_t setup_virtual_context_and_calculate_transition(const gpu_task *task,
																	  const gpu_task *next,
																	  std::vector<Core::context_state_name> &states_to_push,
																	  std::vector<Core::context_state_name> &states_to_pop,
																	  std::vector<Core::context_state> &states_to_set);

private:
	std::function<void(void)> dispatch_func;

public:
	static std::unique_ptr<gpu_state_transition> transition_function(const gpu_task *task, const gpu_task *next);

	gpu_state_transition(const AccessToken&,
						 std::function<void(void)> &&dispatch,
						 unsigned cost,
						 const gpu_task *task,
						 const gpu_task *next) : Base(cost, task, next),
												 dispatch_func(std::move(dispatch)) {}

	void dispatch() const { dispatch_func(); }

	auto get_cost() const { return Base::get_weight(); }
};

}
}
