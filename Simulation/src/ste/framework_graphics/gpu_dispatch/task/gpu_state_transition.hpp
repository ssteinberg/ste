// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "gpu_task.hpp"

#include "gl_virtual_context.hpp"
#include "gl_current_context.hpp"

#include "graph_edge.hpp"

#include <functional>
#include <unordered_map>

#include <memory>

namespace StE {
namespace Graphics {
	
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
	
class gpu_state_transition : public Graph::edge {
private:	
	template <typename K, typename V>
	static std::unordered_map<K,V> states_diff(const std::unordered_map<K,V> &intermediate, const std::unordered_map<K,V> &final_states) {
		std::unordered_map<K,V> diff;
		for (auto &p : intermediate) {
			auto it = final_states.find(p.first);
			if (it == final_states.end()) 
				diff.insert(*it);
		}
		
		return diff;
	}
	
public:
	static auto transition_function(const std::shared_ptr<const gpu_task> &task, const std::shared_ptr<const gpu_task> &next) {
		auto ctx = Core::gl_current_context::get();
		Core::gl_virtual_context virt_ctx;
		virt_ctx.make_current();
		
		task->set_context_state();
		gpu_state_switches switches_intermediate(&virt_ctx);
		auto states_intermediate = virt_ctx.get_states();
		auto resources_intermediate = virt_ctx.get_resources();
		
		next->set_context_state();
		gpu_state_switches transition_switches = gpu_state_switches(&virt_ctx) - switches_intermediate;
		auto states_final = virt_ctx.get_states();
		
		ctx->make_current();
		
		auto diff = states_diff(states_intermediate, states_final);
		transition_switches.total_state_changes += diff.size();
		
		std::function<void(void)> push = [diff]() {
			for (auto &p : diff)
				Core::gl_current_context::get()->push_state(p.first);
		};
		
		std::function<void(void)> transition = [diff = std::move(diff), next]() {
			// Reset states
			for (auto &p : diff)
				Core::gl_current_context::get()->pop_state(p.first);
				
			// Set new states
			next->set_context_state();
		};
		
		return std::make_shared<gpu_state_transition>(std::move(push), 
													  std::move(transition), 
													  transition_switches.cost(), 
													  task, next);
	} 
	
private:
	std::function<void(void)> pre, post;
	unsigned cost;
	
public:
	gpu_state_transition(std::function<void(void)> &&pre, 
						 std::function<void(void)> &&post, 
						 unsigned cost,
						 const std::shared_ptr<const gpu_task> &task, 
						 const std::shared_ptr<const gpu_task> &next) : pre(std::move(pre)),
																		post(std::move(post)), 
																		cost(cost),
																		edge(cost, task.get(), next.get()) {}
	
public:
	void push_states() const { pre(); }
	void transition() const { post(); }
	
	auto get_cost() const { return cost; }
};

}
}
