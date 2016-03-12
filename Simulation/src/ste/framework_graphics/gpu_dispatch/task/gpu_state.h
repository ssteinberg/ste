// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "optional.h"

#include "gl_virtual_context.h"
#include "gl_current_context.h"

#include <functional>
#include <unordered_map>

namespace StE {
namespace Graphics {
	
class gpu_state_transition {
private:
	std::function<void(void)> transition;
	std::size_t cost;
	
public:
	gpu_state_transition(std::function<void(void)> &&f, std::size_t c) : transition(f), cost(c) {}
	
	void operator()() const { transition(); }
	auto get_cost() const { return cost; }
}

class gpu_state {
public:
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
		
		gpu_state_switches(const gl_context_states* ctx) : total_state_changes(ctx->get_total_state_changes()),
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
	}
	
private:
	std::function<void(void)> lambda;
	
	template <typename K, typename V>
	std::unordered_map<K,V> states_diff(const std::unordered_map<K,V> &intermediate, std::unordered_map<K,V> &final_states) {
		std::unordered_map<K,V> diff;
		for (auto &p : intermediate) {
			auto it = final_states.find(p.first);
			if (it != final_states.end()) {
				if (it->second == p.second))
					final_states.erase(it);
			}
			else {
				diff.insert(*it);
			}
		}
		
		return diff;
	}

public:
	gpu_state() = default;
	
	// state_lambda sets the gl context's state. GLSL program, FBO, textures, buffers, states, etc.
	gpu_state(std::function<void(void)> &&state_lambda) : lambda(state_lambda) {}
	
	void operator()() const {
		lambda();
	}
	
	static auto transition_function(const gpu_state &state, const gpu_state &next) {
		auto ctx = gl_current_context::get();
		gl_virtual_context virt_ctx;
		virt_ctx.make_current();
		
		state.lambda();
		gpu_state_switches switches_intermediate(&virt_ctx);
		auto basic_states_intermediate = virt_ctx.get_basic_states();
		auto states_intermediate = virt_ctx.get_states();
		auto resources_intermediate = virt_ctx.get_resources();
		
		next.lambda();
		auto basic_states_final = virt_ctx.get_basic_states();
		auto states_final = virt_ctx.get_states();
		auto resources_final = virt_ctx.get_resources();
		
		gpu_state_switches transition_switches = gpu_state_switches(&virt_ctx) - switches_intermediate;
		auto basic_diff = states_diff(basic_states_intermediate, basic_states_final);
		auto states_diff = states_diff(states_intermediate, states_final);
		states_diff(resources_intermediate, resources_final);
		
		std::function<void(void)> transition = [=]() {
			// Reset states
			for (auto &p : basic_diff) {
				GLenum glname = static_cast<typename std::underlying_type<context_state_name>::type>(p->first);
				p->second.get_old() ?
					glEnable(glname) :
					glDisable(glname);
			}
			for (auto &p : states_diff)
				p->second.get_old_state_setter()();
				
			// Set new states
			for (auto &p : basic_states_final) {
				GLenum glname = static_cast<typename std::underlying_type<context_state_name>::type>(p->first);
				p->second.get_old() ?
					glEnable(glname) :
					glDisable(glname);
			}
			for (auto &p : states_final)
				p->second.get_old_state_setter()();
			for (auto &p : resources_final)
				p->second.get_old_state_setter()();
		};
		
		transition_switches.total_state_changes += basic_diff.size() + states_diff.size();
		std::size_t transition_cost = transition_switches.cost(); 
		
		ctx->make_current();
		
		return gpu_state_transition(std::move(transition), transition_cost);
	} 
};

}
}
