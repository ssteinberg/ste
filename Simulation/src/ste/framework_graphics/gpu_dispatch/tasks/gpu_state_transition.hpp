// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "gpu_task.hpp"

#include "gl_virtual_context.hpp"
#include "gl_current_context.hpp"

#include "sop_edge.hpp"

#include <functional>
#include <unordered_map>

#include <memory>

namespace StE {
namespace Graphics {
	
class gpu_state_transition;

namespace _gpu_state_transition_impl {
	
class gpu_state_switches {
private:
	static constexpr unsigned state_change_cost = 1;
	static constexpr unsigned buffer_change_cost = 3;
	static constexpr unsigned texture_change_cost = 20;
	static constexpr unsigned shader_change_cost = 15;
	static constexpr unsigned fbo_change_cost = 5;
	static constexpr unsigned va_change_cost = 10;

public:
	std::size_t total_state_changes;
	std::size_t total_buffer_changes;
	std::size_t total_texture_changes;
	std::size_t total_shader_changes;
	std::size_t total_fbo_changes;
	std::size_t total_va_changes;
	
	gpu_state_switches() = default;
	gpu_state_switches(const Core::gl_virtual_context* ctx) : total_state_changes(ctx->get_total_state_changes()),
															  total_buffer_changes(ctx->get_total_buffer_changes()),
															  total_texture_changes(ctx->get_total_texture_changes()),
															  total_shader_changes(ctx->get_total_shader_changes()),
															  total_fbo_changes(ctx->get_total_fbo_changes()),
															  total_va_changes(ctx->get_total_va_changes()) {}
													
	std::size_t cost() const {
		return total_state_changes * state_change_cost +
			total_buffer_changes * buffer_change_cost +
			total_texture_changes * texture_change_cost +
			total_shader_changes * shader_change_cost +
			total_fbo_changes * fbo_change_cost +
			total_va_changes * va_change_cost;
	}
	
	gpu_state_switches operator-(const gpu_state_switches &s) const {
		gpu_state_switches n;
		n.total_state_changes = total_state_changes - s.total_state_changes;
		n.total_buffer_changes = total_buffer_changes - s.total_buffer_changes;
		n.total_texture_changes = total_texture_changes - s.total_texture_changes;
		n.total_shader_changes = total_shader_changes - s.total_shader_changes;
		n.total_fbo_changes = total_fbo_changes - s.total_fbo_changes;
		n.total_va_changes = total_va_changes - s.total_va_changes;
		
		return n;
	}
};

}
	
class gpu_state_transition : public Algorithm::SOP::sop_edge {
	using Base = Algorithm::SOP::sop_edge;
	
private:
	struct AccessToken {};
	
private:
	template <typename K, typename V>
	static bool states_diff(std::unordered_map<K,V> &&intermediate, const std::unordered_map<K,V> &final_states, std::vector<K> &states_to_push, std::vector<K> &states_to_pop) {
		auto max_count = std::max(intermediate.size(), final_states.size());
		if (max_count == 0)
			return false;

		states_to_push.reserve(max_count);
		states_to_pop.reserve(max_count);
		
		for (auto &p : final_states) {
			auto it = intermediate.find(p.first);
			if (it == intermediate.end()) 
				states_to_push.push_back(p.first);
			else
				intermediate.erase(it);
		}
		
		for (auto &p : intermediate)
			states_to_pop.push_back(p.first);
		
		return true;
	}
	
protected:
	static Core::gl_virtual_context virt_ctx;
	
public:
	static auto transition_function(const gpu_task *task, const gpu_task *next) {
		auto ctx = Core::gl_current_context::get();
		virt_ctx.make_current();
		
		virt_ctx.clear();
		task->set_context_state();
		auto states_intermediate = virt_ctx.get_states();
		
		virt_ctx.clear();
		next->set_context_state();
		
		std::vector<Core::context_state_name> states_to_pop, states_to_push;
		states_diff(std::move(states_intermediate), virt_ctx.get_states(), states_to_push, states_to_pop);
		
		_gpu_state_transition_impl::gpu_state_switches transition_switches(&virt_ctx);
		transition_switches.total_state_changes += states_to_pop.size();
		
		ctx->make_current();
		
		std::function<void(void)> dispatch = [states_to_push = std::move(states_to_push), states_to_pop = std::move(states_to_pop), task]() {				
			// Push states used by task 
			for (auto &k : states_to_push)
				Core::gl_current_context::get()->push_state(k);
				
			// Set new states
			task->set_context_state();
			// Dispatch
			task->dispatch();
			
			// Reset states
			for (auto &k : states_to_pop)
				Core::gl_current_context::get()->pop_state(k);
		};
		
		return std::make_unique<gpu_state_transition>(AccessToken(),
													  std::move(dispatch),
													  transition_switches.cost(), 
													  task, next);
	} 
	
private:
	std::function<void(void)> dispatch_func;
	
public:
	gpu_state_transition(const AccessToken&,
						 std::function<void(void)> &&dispatch, 
						 unsigned cost,
						 const gpu_task *task, 
						 const gpu_task *next) : Base(cost, task, next),
												 dispatch_func(std::move(dispatch)) {}
	
public:
	void dispatch() const { dispatch_func(); }
	
	auto get_cost() const { return Base::get_weight(); }
};

}
}
