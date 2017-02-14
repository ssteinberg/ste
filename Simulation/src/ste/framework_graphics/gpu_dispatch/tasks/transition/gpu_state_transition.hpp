// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <gpu_task.hpp>
#include <sop_edge.hpp>

#include <gl_context_state_log.hpp>

#include <functional>
#include <vector>
#include <unordered_map>

#include <memory>

namespace StE {
namespace Graphics {

class gpu_state_transition : public Algorithm::SOP::sop_edge {
	using Base = Algorithm::SOP::sop_edge;

private:
	void log_and_set_task_state(const gpu_task *task) const;
	Core::GL::gl_context_state_log state_to_pop(const gpu_task *prev_task, const gpu_task *task) const;

public:
	gpu_state_transition(const gpu_task *task,
						 const gpu_task *next) : Base(task, next) {}

	void update_weight_and_transition() const override final;
};

}
}
